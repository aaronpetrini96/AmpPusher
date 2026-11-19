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
    warm_up = 200

    depths = []
    for group in np.array_split(np.arange(len(wet)), 3):
        group_wet = wet[group]
        dry_batch = np.repeat(dry[None, :, None], len(group), axis=0)
        condition_batch = np.repeat(dataset.conditions[group][:, None, :], len(dry), axis=1)
        inputs = torch.from_numpy(np.concatenate([dry_batch, condition_batch], axis=2))
        with torch.no_grad():
            predictions = model(inputs)[..., 0].numpy()
        residue = predictions[:, warm_up:] - group_wet[:, warm_up:]
        signal_power = np.sum(group_wet[:, warm_up:] ** 2, axis=1)
        depths.extend(10 * np.log10(signal_power / np.sum(residue**2, axis=1)))

    return [(drive, tone, depth) for (drive, tone), depth in zip(dataset.conditions, depths)]


def main() -> None:
    """Prints the null depth table for one pedal."""
    arguments = parse_arguments()
    print(f"{'drive':>6} {'tone':>6} {'null (dB)':>10}")
    for drive, tone, depth in null_depths(arguments.pedal):
        print(f"{drive:>6.2f} {tone:>6.2f} {depth:>10.1f}")


if __name__ == "__main__":
    main()
