"""Conditioned GRU pedal model: audio plus knob values in, processed audio out."""

import torch
from torch import nn


class PedalNet(nn.Module):
    """A single-layer GRU followed by a linear output, matching RTNeural's GRULayerT and DenseT."""

    def __init__(self, input_size: int = 3, hidden_size: int = 16) -> None:
        super().__init__()
        self.gru = nn.GRU(input_size=input_size, hidden_size=hidden_size, batch_first=True)
        self.output = nn.Linear(hidden_size, 1)

    def forward(self, inputs: torch.Tensor) -> torch.Tensor:
        """Maps (chunks, samples, input_size) to (chunks, samples, 1)."""
        states, _ = self.gru(inputs)
        return self.output(states)
