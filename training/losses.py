"""Training loss: pre-emphasised error-to-signal ratio (ESR)."""

import torch


def pre_emphasis(signal: torch.Tensor, coefficient: float = 0.85) -> torch.Tensor:
    """First-order highpass along time, so high-frequency errors weigh more in the loss."""
    return signal[:, 1:] - coefficient * signal[:, :-1]


def esr(prediction: torch.Tensor, target: torch.Tensor, warm_up: int) -> torch.Tensor:
    """Pre-emphasised ESR over the whole batch, skipping each chunk's first warm_up samples."""
    prediction = pre_emphasis(prediction[:, warm_up:])
    target = pre_emphasis(target[:, warm_up:])
    return torch.mean((prediction - target) ** 2) / torch.mean(target ** 2)