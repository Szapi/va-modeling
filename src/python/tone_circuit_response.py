import sympy as sp
import numpy as np
import matplotlib.pyplot as plt

# Elementary symbols
s, Rp, Tp, Zf, Zin = sp.symbols('s Rp Tp Zf Zin', real=True, positive=True)

# Compound expressions
Rp1 = (0.99998 * Tp + 0.00001) * Rp         # Hack: avoid division by zero
Rp2 = (1 - (0.99998 * Tp + 0.00001)) * Rp   # Hack: avoid division by zero
Zp = 220 + (1 / (s * 220e-9))
X = (Zp * Rp) / (Zp * Rp + Rp1 * Rp2)
Zs = 1 / (s * 220e-9 + (1 / 10000))

# The transfer function
H = (((1 - X)/Rp2) + (1/Zf))   /   (((1 - X)/Rp1) + (1/Zs) + (1/Zin))

# Substitute component values
Rp_val = 20000 # 20k Ohm
Zf_val = 1000  # 1k Ohm
Zin_val = 1000  # 1k Ohm

# Define frequency range
frequencies = np.logspace(1, 5, 1000)  # 10 Hz to 100 kHz
omega = 2 * np.pi * frequencies
s_vals = 1j * omega

def evaluate(Tp_val):
    H_num = H.subs({Rp: Rp_val, Tp: Tp_val, Zf : Zf_val, Zin: Zin_val})
    # Convert symbolic function to a numpy-compatible one
    H_func = sp.lambdify(s, H_num, modules='numpy')
    # Evaluate transfer function
    H_vals = H_func(s_vals)
    # Magnitude in dB
    magnitude_db = 20 * np.log10(np.abs(H_vals))
    # Phase in degrees
    phase_deg = np.angle(H_vals, deg=True)
    return magnitude_db, phase_deg


def plot_transfer_function(Tp_vals):
    curves = []
    for tp in Tp_vals:
        m,p = evaluate(tp)
        curves.append([tp,m,p])

    # Plot magnitudes
    plt.figure(figsize=(10, 6))
    plt.subplot(2, 1, 1)
    for tmp in curves:
        plt.semilogx(frequencies, tmp[1], label=f"Tone = {tmp[0]}")
    plt.title('Frequency Response')
    plt.ylabel('Magnitude (dB)')
    plt.legend()
    plt.grid(True, which='both')

    # Plot phase
    plt.subplot(2, 1, 2)
    for tmp in curves:
        plt.semilogx(frequencies, tmp[2], label=f"Tone = {tmp[0]}")
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Phase (degrees)')
    plt.grid(True, which='both')

    plt.tight_layout()
    plt.legend()
    plt.show()

plot_transfer_function([0.0, 0.5, 1.0])
