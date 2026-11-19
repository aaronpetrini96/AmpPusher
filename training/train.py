import argparse

import numpy as np
import torch

from training.dataset import PedalDataset
from training.losses import esr
from training.model import PedalNet
from training.pedals import PEDAL_DIRS, REPO_ROOT

MODELS_DIR = REPO_ROOT / "models"


def parse_arguments() -> argparse.Namespace:
    """Command-line options; anything not given keeps its default."""
    parser = argparse.ArgumentParser(description="Train a conditioned GRU pedal model.")
    parser.add_argument("pedal", choices=sorted(PEDAL_DIRS))
    parser.add_argument("--hidden-size", type=int, default=16)
    parser.add_argument("--steps", type=int, default=2000)
    parser.add_argument("--learning-rate", type=float, default=0.005)
    parser.add_argument("--chunk-count", type=int, default=32)
    parser.add_argument("--chunk-length", type=int, default=2000)
    parser.add_argument("--warm-up", type=int, default=200)
    parser.add_argument("--seed", type=int, default=0)
    return parser.parse_args()


def main() -> None:
    """Trains the pedal's model and saves the checkpoint with the best validation loss."""
    arguments = parse_arguments()
    torch.manual_seed(arguments.seed)
    rng = np.random.default_rng(arguments.seed)

    dataset = PedalDataset.load(arguments.pedal)
    validation_inputs, validation_targets = dataset.batch(
        np.random.default_rng(999), 64, arguments.chunk_length, validation=True
    )

    model = PedalNet(hidden_size=arguments.hidden_size)
    optimizer = torch.optim.Adam(model.parameters(), lr=arguments.learning_rate)

    MODELS_DIR.mkdir(exist_ok=True)
    checkpoint_path = MODELS_DIR / f"{arguments.pedal}.pt"
    best_validation_loss = float("inf")

    for step in range(arguments.steps + 1):
        inputs, targets = dataset.batch(rng, arguments.chunk_count, arguments.chunk_length)
        loss = esr(model(inputs), targets, arguments.warm_up)
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

        if step % 500 == 0:
            with torch.no_grad():
                validation_loss = esr(model(validation_inputs), validation_targets, arguments.warm_up).item()
            if validation_loss < best_validation_loss:
                best_validation_loss = validation_loss
                torch.save({"state_dict": model.state_dict(), "config": vars(arguments)}, checkpoint_path)
            print(
                f"step {step}: training {loss.item():.5f}, validation {validation_loss:.5f}, best {best_validation_loss:.5f}"
            )


if __name__ == "__main__":
    main()
