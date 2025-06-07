import soundfile as sf
from scipy.signal import resample_poly

def downsample_wav(input_path, output_path, downsample_factor=8):
    # Read the high-rate WAV file
    data, samplerate = sf.read(input_path)
    
    if samplerate != 1536000:
        raise ValueError(f"Expected 1.536 MHz input, got {samplerate} Hz")
    
    # Downsample using polyphase filtering (automatically includes antialiasing)
    # <https://dsp.stackexchange.com/q/45446>
    data_downsampled = resample_poly(data, up=1, down=downsample_factor)

    # Save to new WAV file at 192 kHz
    sf.write(output_path, data_downsampled, samplerate // downsample_factor)
    print(f"Downsampled WAV written to: {output_path}")

# Example usage
#if __name__ == "__main__":
    #downsample_wav("1536.wav", "192.wav")
