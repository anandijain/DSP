from scipy.io import wavfile
import os.path
import numpy


save_path = r'C:\Users\Anand\Programming\wav'


def wav_print(file_name, write_file_name):

	numpy.set_printoptions(threshold=numpy.nan)
	fs, data = wavfile.read(os.path.join(save_path, file_name + ".wav"))
	print(fs)
	print(str(data))
	full_write_path = os.path.join(save_path, write_file_name + ".txt")
	file = open(full_write_path, "a", encoding="utf-8")
	file.write(str(data))
	file.close()
	return


wav_print("sound_test3", "sound_test3_text")
