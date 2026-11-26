"""Loads a pedal's renders and cuts random chunks for training and validation."""

import itertools
from dataclasses import dataclass
from pathlib import Path

import numpy as np
import soundfile as sf
import torch

from training.pedals import (
    DRIVE_SETTINGS,
    INPUT_FILE,
    TONE_SETTINGS,
    VALIDATION_FRACTION,
    di_start_sample,
    render_file,
)


def _read_mono(path: Path) -> np.ndarray:
    """First channel of an audio file as float32, for mono and stereo files alike."""
    audio, _ = sf.read(path, dtype="float32", always_2d=True)
    return audio[:, 0]


@dataclass
class PedalDataset:
    """All renders of one pedal, aligned with the dry input, split into training and validation regions."""

    dry: np.ndarray
    wet: np.ndarray
    conditions: np.ndarray
    validation_start: int

    @classmethod
    def load(cls, pedal: str) -> "PedalDataset":
        """Reads the input file and every render of the pedal, trimmed to a common length."""
        settings = list(itertools.product(DRIVE_SETTINGS, TONE_SETTINGS))
        renders = [_read_mono(render_file(pedal, drive, tone)) for drive, tone in settings]
        dry = _read_mono(INPUT_FILE)

        length = min(len(dry), *(len(render) for render in renders))
        di_start = di_start_sample()
        validation_start = di_start + int((length - di_start) * (1.0 - VALIDATION_FRACTION))

        trimmed_dry = dry[:length]
        trimmed_wet = np.stack([render[:length] for render in renders])
        conditions = np.array(settings, dtype=np.float32)
        return cls(trimmed_dry, trimmed_wet, conditions, validation_start)

    def batch(
        self, rng: np.random.Generator, chunk_count: int, chunk_length: int, validation: bool = False
    ) -> tuple[torch.Tensor, torch.Tensor]:
        """Random chunks as model inputs (chunks, samples, 3) = [audio, drive, tone] and targets (chunks, samples, 1)."""
        first, last = (self.validation_start, len(self.dry)) if validation else (0, self.validation_start)
        starts = rng.integers(first, last - chunk_length, size=chunk_count)
        settings = rng.integers(0, len(self.wet), size=chunk_count)

        dry_chunks = np.stack([self.dry[start : start + chunk_length] for start in starts])
        wet_chunks = np.stack(
            [self.wet[setting, start : start + chunk_length] for setting, start in zip(settings, starts)]
        )
        condition_chunks = np.repeat(self.conditions[settings][:, None, :], chunk_length, axis=1)

        inputs = np.concatenate([dry_chunks[..., None], condition_chunks], axis=2)
        targets = wet_chunks[..., None]
        return torch.from_numpy(inputs), torch.from_numpy(targets)
