from sklearn.cluster import SpectralClustering
import torch
import torch.nn.functional as F
import numpy as np

@torch.no_grad()
def spectral_clustering(feats: torch.Tensor, n_clusters = 2, tau: float = 0, eps: float = 1e-5):
    assert feats.ndim == 3, feats.shape

    feats = F.normalize(feats, p=2, dim=1)

    A = feats.transpose(1, 2) @ feats

    A[A < tau] = eps

    labels = np.stack([
        SpectralClustering(n_clusters=n_clusters, affinity="precomputed").fit_predict(Ai) for Ai in A
    ])

    return labels

    