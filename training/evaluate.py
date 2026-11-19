"""Measures how closely each trained model nulls against its pedal on unseen audio."""

import argparse

import numpy as np
import torch

from training.dataset import PedalDataset
from training.export import load_model
from training.pedals import PEDAL_DIRS


def parse_arguments() -> argparse.Namespace:
    """Command-line options."""
    parser = argparse.ArgumentParser(description="Null depth of a trained pedal model per knob setting.")
    parser.add_argument("pedal", choices=sorted(PEDAL_DIRS))
    return parser.parse_args()


def null_depths(pedal: str) -> list[tuple[float, float, float]]:
    """Null depth in dB of the model against each render, on the validation region (unseen DI)."""
    dataset = PedalDataset.load(pedal)
    model, _ = load_model(pedal)

    dry = dataset.dry[dataset.validation_start :]
    wet = dataset.wet[:, dataset.validation_start :]
    setting_count, sample_count = wet.shape

    dry_batch = np.repeat(dry[None, :, None], setting_count, axis=0)
    condition_batch = np.repeat(dataset.conditions[:, None, :], sample_count, axis=1)
    inputs = torch.from_numpy(np.concatenate([dry_batch, condition_batch], axis=2))

    with torch.no_grad():
        predictions = model(inputs)[..., 0].numpy()

    warm_up = 200
    residue = predictions[:, warm_up:] - wet[:, warm_up:]
    signal_power = np.sum(wet[:, warm_up:] ** 2, axis=1)
    residue_power = np.sum(residue**2, axis=1)
    depths = 10 * np.log10(signal_power / residue_power)
    return [(drive, bright, depth) for (drive, bright), depth in zip(dataset.conditions, depths)]


def main() -> None:
    """Prints the null depth table for one pedal."""
    arguments = parse_arguments()
    print(f"{'drive':>6} {'bright':>7} {'null (dB)':>10}")
    for drive, bright, depth in null_depths(arguments.pedal):
        print(f"{drive:>6.2f} {bright:>7.0f} {depth:>10.1f}")


if __name__ == "__main__":
    main()
