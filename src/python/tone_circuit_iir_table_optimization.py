import sympy as sp
import numpy as np

from scipy.optimize import minimize
from scipy.signal import freqz

from itertools import combinations
from collections import defaultdict

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

from decimal import Decimal, getcontext

getcontext().prec = 17

def CalculateTargetResponse(freqs_Hz, Tp_val):
    ###########################################################################
    ## Calculate the desired response
    # Elementary symbols
    s, Rp, Tp, Zf, Zin = sp.symbols('s Rp Tp Zf Zin', real=True, positive=True)

    # Compound expressions
    Rp1 = (0.99998 * Tp + 0.00001) * Rp
    Rp2 = (1 - (0.99998 * Tp + 0.00001)) * Rp
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
    omega = 2 * np.pi * freqs_Hz
    s_vals = 1j * omega

    H_num = H.subs({Rp: Rp_val, Tp: Tp_val, Zf : Zf_val, Zin: Zin_val})

    # Convert symbolic function to a numpy-compatible one
    H_func = sp.lambdify(s, H_num, modules='numpy')

    # Evaluate transfer function
    H_target = H_func(s_vals)
    ###########################################################################
    return H_target


# Parameters
fs = 192000  # Sampling rate


def GetFreqBins():
    # Generate log-spaced frequencies in Hz, then normalize to radians/sample
    f_min = 40
    f_max = fs / 2
    N_bins = 2048
    freqs_Hz = np.logspace(np.log10(f_min), np.log10(f_max), N_bins)
    w = 2 * np.pi * freqs_Hz / fs  # Normalized digital frequency [rad/sample]
    return freqs_Hz, w


freqs_Hz, freqs_norm = GetFreqBins()

###########################################################################
## Weights for the error function to penalize deviations from the target
## response in the audible frequency range
weights = np.ones_like(freqs_Hz)
weights[freqs_Hz > 20000] = 0.2
#print("Weights: ", weights)
###########################################################################

num_iir_params = 5
## The 5 parameters of the iir will be stored next to each other in arrays, like this:
i_b0 = 0
i_b1 = 1
i_b2 = 2
i_a1 = 3
i_a2 = 4

## Calculates error for the IIR frequency response vs a target curve
def iir_response_error(params, H_target):
    b = [params[i_b0], params[i_b1], params[i_b2]]
    a = [1.0, params[i_a1], params[i_a2]]
    _, h = freqz(b, a, worN=freqs_norm)
    error = weights * np.abs(h - H_target)**2
    error = np.sum(error * 10.0)
    zeros = np.roots(b)
    poles = np.roots(a)
    # Penalize poles outside the unit circle
    pole_penalty = np.sum( (1.01 * (np.abs(poles) + 0.01))**64 )
    # Penalize non-minimum-phase zeros
    zero_penalty = np.sum( (1.01 * (np.abs(zeros) + 0.01))**64 )
    return error + pole_penalty + zero_penalty


def OptimizeIIR(H_target):
    # Initial guess
    x0 = [0.3, 0.1, 0.1, -0.3, 0.1]
    # Run optimization
    res = minimize(iir_response_error, x0, args=(H_target), method='BFGS')
    # Extract coefficients
    return res.x


def GenerateInitialIIRs(H_targets):
    # result = np.array([])
    # for H in H_targets:
    #     res = OptimizeIIR(H)
    #     result = np.append(result, res)
    # This is already an optimal solution
    result = [
0.01033164544936915, -0.0029498174269666994, -0.006564671835362914, -1.9103944261075956, 0.9112885306110365,
0.013323263506474427, -0.003962417685130555, -0.008584489225247714, -1.9108796868776832, 0.9117358805992783,
0.014924737446949693, -0.00458803097978175, -0.009716170051257031, -1.9231567452227505, 0.9238404169614899,
0.014549799337115624, -0.004584716748644842, -0.009545172436183885, -1.9398684197611173, 0.9403288797162197,
0.01456190077440002, -0.004650263870455205, -0.009578676731396663, -1.9474998095034477, 0.9478678651528998,
0.01460075025452719, -0.004692295194540307, -0.00962320679141882, -1.9516397896844595, 0.9519546982038353,
0.014797659755021017, -0.004786454565941298, -0.009766405601820237, -1.9557794728464335, 0.9560507153468053,
0.014941329710919389, -0.004845860239521635, -0.009869652260641461, -1.9585755914458292, 0.9588271925637211,
0.015027930928788982, -0.004871709987884541, -0.009925330912157535, -1.9604812886749439, 0.9607385886935808,
0.015236259618748886, -0.004857300361386576, -0.010005138345844777, -1.955877396170959, 0.9562922712370195,
0.015337206346739853, -0.004896912696254915, -0.010066805567629662, -1.9570651528494396, 0.957475958339106,
0.015543378713824857, -0.004978737370823467, -0.01021122433052999, -1.9588201048858256, 0.9592056593016018,
0.016049002137382497, -0.005165514714630577, -0.010545671873790673, -1.9602870406446238, 0.9606527759747189,
0.017058963682736957, -0.005547066265408002, -0.011246899350846827, -1.963368465901883, 0.9636561092197956,
0.01829750425254333, -0.005963040384086325, -0.012067457681796961, -1.9631326217446474, 0.9634239482084369,
0.019984267704877923, -0.006516065007825363, -0.013168879447103411, -1.9614326475089756, 0.9617608924762868,
0.02142069878708475, -0.006964209931182292, -0.014102751169285605, -1.9591242771522246, 0.9595120363744957,
0.02353439997572625, -0.007612916619665554, -0.01548458773209695, -1.9556635740779331, 0.9561409847281481,
0.02731136010544338, -0.008794178918624024, -0.01794150917391501, -1.9496965552534193, 0.9503285537923496,
0.03580915279183702, -0.011439309385287, -0.023457764268009416, -1.935807537212602, 0.9368105352542964,
0.07349140521380336, -0.023182704264613014, -0.04788879384003686, -1.8740761062978415, 0.8767392999769166,
    ]
    return result

# given 2-2 complex numbers, we return the minimum distance between
# the vector pairing ([z1, z2] and [u1, u2]) OR ([z1, z2] and [u2, u1])
def least_distance(z1, z2, u1, u2):
    d1 = np.abs(z1 - u1) + np.abs(z2 - u2)
    d2 = np.abs(z1 - u2) + np.abs(z2 - u1)
    return min(d1, d2)


# Convert the 5-5 IIR parameters to poles and zeros, and calculate the least distance
# between the two sets of poles and zeros
def distance(p, q):
    zeros_a = np.roots([p[0], p[1], p[2]])
    poles_a = np.roots([1.0, p[3], p[4]])
    zeros_b = np.roots([q[0], q[1], q[2]])
    poles_b = np.roots([1.0, q[3], q[4]])
    # Calculate the least distance between the two sets of poles and zeros
    d1 = least_distance(zeros_a[0], zeros_a[1], zeros_b[0], zeros_b[1])
    d2 = least_distance(poles_a[0], poles_a[1], poles_b[0], poles_b[1])
    return d1 + d2


cnt = 0
def OptimizeIIR_Sweep(Tp_vals):
    H_targets = [CalculateTargetResponse(freqs_Hz, Tp) for Tp in Tp_vals]

    def iir_sweep_error(params):
        def ith(i):
            return params[i*num_iir_params : (i+1)*num_iir_params]
        def every_ith(i):
            return params[i : len(params) : num_iir_params]

        error = 0.0
        ## Error from frequency responses
        for i, H in enumerate(H_targets):
            error += iir_response_error(ith(i), H)
        error_fr = error
        ## Error from pole-zero distances
        for i, _ in enumerate(H_targets):
            if i != 0:
                error += 100.0 * distance(ith(i), ith(i-1))
        global cnt
        if ((cnt % 100) == 0):
            print(f"Error from frequency responses: {error_fr}, Final error: {error}")
            print(np.array2string(params, separator=", "))
        cnt += 1
        return error

    x0 = GenerateInitialIIRs(H_targets)
    # Run optimization
    res = minimize(iir_sweep_error, x0, method='BFGS')
    return res.x, H_targets

TonePot_vals = [0.00, 0.02, 0.04, 0.06, 0.08, 0.1, 0.14, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.86, 0.9, 0.92, 0.94, 0.96, 0.98, 1.0]

res, H_targets = OptimizeIIR_Sweep(TonePot_vals)
iir_coeffs = []
print("   ---   IIR Coefficients: [")
for i in range(len(TonePot_vals)):
    params = res[i*5 : (i+1)*5]
    iir_coeffs.append(tuple(params))
    print(f"{params[0]}, {params[1]}, {params[2]}, {params[3]}, {params[4]},")
print("]  ---")

print(iir_coeffs)

# Pre-allocate response surface
response_surface = np.zeros((len(TonePot_vals), len(freqs_Hz)))
# --- Pre-allocate phase surface ---
phase_surface = np.zeros((len(TonePot_vals), len(freqs_Hz)))

print("Calculating frequency responses...")
for i, Tp_val in enumerate(TonePot_vals):
    H = CalculateTargetResponse(freqs_Hz, Tp_val)
    response_surface[i, :] = np.abs(H)  # Take magnitude for plotting
    phase_surface[i, :] = np.angle(H, deg=True)  # Phase in degrees

# Pre-allocate IIR response surfaces
iir_mag_surface = np.zeros_like(response_surface)
iir_phase_surface = np.zeros_like(phase_surface)
# Evaluate IIR filters
print("Calculating IIR filter responses...")
for i, (b0, b1, b2, a1, a2) in enumerate(iir_coeffs):
    b = [b0, b1, b2]
    a = [1.0, a1, a2]
    w_iir, h_iir = freqz(b, a, worN=freqs_norm)
    iir_mag_surface[i, :] = np.abs(h_iir)
    iir_phase_surface[i, :] = np.angle(h_iir, deg=True)

# --- Prepare meshgrid for 3D plotting ---
FREQS, TPS = np.meshgrid(freqs_Hz, TonePot_vals)

# --- Plotting ---
fig = plt.figure(figsize=(12, 8))
ax = fig.add_subplot(111, projection='3d')

# Use log scale for frequency axis
surf = ax.plot_surface(np.log10(FREQS), TPS, response_surface, cmap='viridis', edgecolor='none')

# Plot IIR magnitude on top
surf_iir = ax.plot_surface(np.log10(FREQS), TPS, iir_mag_surface,
                           cmap='inferno', alpha=0.6, edgecolor='none')

ax.set_xlabel('log10(Frequency) [Hz]')
ax.set_ylabel('Tp value')
ax.set_zlabel('|H(f)|')
ax.set_title('Frequency Response Surface vs Tp')

fig.colorbar(surf, shrink=0.5, aspect=10, label='|H(f)|')
fig.colorbar(surf_iir, shrink=0.5, aspect=10, label='IIR |H(f)|')
# --- Plotting ---

# --- Phase Plotting ---
fig2 = plt.figure(figsize=(12, 8))
ax2 = fig2.add_subplot(111, projection='3d')

# Plot surface
surf2 = ax2.plot_surface(np.log10(FREQS), TPS, phase_surface, cmap='twilight', edgecolor='none')
surf_iir_phase = ax2.plot_surface(np.log10(FREQS), TPS, iir_phase_surface,
                                  cmap='coolwarm', alpha=0.6, edgecolor='none')

ax2.set_xlabel('log10(Frequency) [Hz]')
ax2.set_ylabel('Tp value')
ax2.set_zlabel('Phase [deg]')
ax2.set_title('Phase Response Surface vs Tp')

fig2.colorbar(surf2, shrink=0.5, aspect=10, label='Phase [deg]')
fig2.colorbar(surf_iir_phase, shrink=0.5, aspect=10, label='IIR Phase')
# --- Phase Plotting ---

# Unpack the coefficient arrays
b0_vals, b1_vals, b2_vals, a1_vals, a2_vals = zip(*iir_coeffs)

# Plotting
fig, axes = plt.subplots(5, 1, figsize=(10, 12), sharex=True)

coeff_names = ['b0', 'b1', 'b2', 'a1', 'a2']
coeff_values = [b0_vals, b1_vals, b2_vals, a1_vals, a2_vals]

for ax, name, values in zip(axes, coeff_names, coeff_values):
    ax.plot(TonePot_vals, values, marker='o', linestyle='-', label=name)
    ax.set_ylabel(name)
    ax.grid(True)
    ax.legend(loc='best')

axes[-1].set_xlabel('Tone parameter')
fig.suptitle('IIR Coefficient Sweep vs Control Parameter', fontsize=16)
plt.tight_layout(rect=[0, 0, 1, 0.96])

plt.tight_layout()
plt.show()
