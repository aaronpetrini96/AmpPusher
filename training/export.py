"""Exports a trained pedal model to JSON for RTNeural, plus a parity clip for the C++ test."""

import argparse
import json

import numpy as np
import torch

from training.dataset import PedalDataset
from training.model import PedalNet
from training.pedals import PEDAL_DIRS, REPO_ROOT, SAMPLE_RATE

MODELS_DIR = REPO_ROOT / "models"
PARITY_DIR = REPO_ROOT / "tests" / "data"


def parse_arguments() -> argparse.Namespace:
    """Command-line options."""
    parser = argparse.ArgumentParser(description="Export a trained pedal model for the plugin.")
    parser.add_argument("pedal", choices=sorted(PEDAL_DIRS))
    return parser.parse_args()


def load_model(pedal: str) -> tuple[PedalNet, dict]:
    """Rebuilds the trained model from its checkpoint and returns it with its training options."""
    checkpoint = torch.load(MODELS_DIR / f"{pedal}.pt")
    config = checkpoint["config"]
    model = PedalNet(hidden_size=config["hidden_size"])
    model.load_state_dict(checkpoint["state_dict"])
    model.eval()
    return model, config


def export_weights(model: PedalNet, config: dict, pedal: str) -> None:
    """Writes the weights as JSON in PyTorch's naming, which RTNeural's torch_helpers load directly."""
    weights = {name: tensor.tolist() for name, tensor in model.state_dict().items()}
    weights["hidden_size"] = config["hidden_size"]
    weights["sample_rate"] = SAMPLE_RATE
    with open(MODELS_DIR / f"{pedal}.json", "w") as file:
        json.dump(weights, file)


def export_parity(model: PedalNet, pedal: str) -> None:
    """Writes 100 ms of unseen input and the model's output, for the C++ parity test."""
    dataset = PedalDataset.load(pedal)
    test_length = SAMPLE_RATE // 10
    test_dry = dataset.dry[dataset.validation_start : dataset.validation_start + test_length]
    test_conditions = np.tile([1.0, 1.0], (test_length, 1)).astype(np.float32)
    model_input = np.concatenate([test_dry[:, None], test_conditions], axis=1)
    with torch.no_grad():
        expected = model(torch.from_numpy(model_input[None]))[0, :, 0].numpy()

    PARITY_DIR.mkdir(parents=True, exist_ok=True)
    with open(PARITY_DIR / f"{pedal}_parity.json", "w") as file:
        json.dump({"inputs": model_input.tolist(), "expected": expected.tolist()}, file)


def main() -> None:
    """Exports the pedal's weights and parity clip."""
    arguments = parse_arguments()
    model, config = load_model(arguments.pedal)
    export_weights(model, config, arguments.pedal)
    export_parity(model, arguments.pedal)


if __name__ == "__main__":
    main()
