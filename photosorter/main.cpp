#include <iostream>
#include <fstream>
using namespace std;

void flip4(char*);
void flip2(char*);
int int4byte(char*);
int int2byte(char*);
int ascii4(char*);
int ascii2(char*);

int main() {
	string nomeFile = "C:\\Users\\cappe\\Desktop\\a.jpg";
	//cout << "Nome file: ";
	//cin >> nomeFile;
	ifstream immagine;
	immagine.open(nomeFile, ios::binary);
	if (immagine.is_open()) {
		int anno, mese;
		int posizione;
		int ncampi;
		bool bigEndian = true;
		char temp4byte[4];
		char temp2byte[2];
		
		//controlla exif
		immagine.seekg(6);
		immagine.read(temp4byte, 4);
		if (strncmp(temp4byte, "Exif", 4) == 0) {
			//controlla endian
			immagine.seekg(12);
			immagine.read(temp2byte, 2);
			if (strncmp(temp2byte, "II", 2) == 0) {
				bigEndian = false;
			}
			
			//IFD
			immagine.seekg(16);
			immagine.read(temp4byte, 4);
			if (!bigEndian) flip4(temp4byte);
			int posizione = 12 + int4byte(temp4byte); 
			immagine.seekg(posizione);
			immagine.read(temp2byte, 2);
			if (!bigEndian) flip2(temp2byte);
			ncampi = int2byte(temp2byte);
			posizione += 2;
			bool DateTimeTrovato = false;
			bool ExifIFDPointerTrovato = false;
			int ExifIFDPointer;
			for (int i = 0; i < ncampi; i++) {
				immagine.seekg(posizione);
				immagine.read(temp2byte, 2);
				if (!bigEndian) flip2(temp2byte);
				if (strncmp(temp2byte, "\x01\x32", 2) == 0) {
					DateTimeTrovato = true;
					posizione += 8;
					immagine.seekg(posizione);
					immagine.read(temp4byte, 4);
					if (!bigEndian) flip4(temp4byte);
					posizione = 12 + int4byte(temp4byte);
					immagine.seekg(posizione);
					immagine.read(temp4byte, 4);
					anno = ascii4(temp4byte);
					posizione += 5;
					immagine.seekg(posizione);
					immagine.read(temp2byte, 2);
					mese = ascii2(temp2byte);
					break;
				}

				if (ExifIFDPointerTrovato == false && strncmp(temp2byte, "\x87\x69", 2) == 0 ) {
					ExifIFDPointerTrovato = true;
					posizione += 8;
					immagine.seekg(posizione);
					immagine.read(temp4byte, 4);
					if (!bigEndian) flip4(temp4byte);
					ExifIFDPointer = int4byte(temp4byte);
					posizione -= 8;
				}
				posizione += 12;
			}

			if (DateTimeTrovato == false && ExifIFDPointerTrovato == true) {
				posizione = 12 + ExifIFDPointer;
				immagine.seekg(posizione);
				immagine.read(temp2byte, 2);
				if (!bigEndian) flip2(temp2byte);
				ncampi = int2byte(temp2byte);
				posizione += 2;

				for (int i = 0; i < ncampi; i++) {
					immagine.seekg(posizione);
					immagine.read(temp2byte, 2);
					if (!bigEndian) flip2(temp2byte);
					if (strncmp(temp2byte, "\x90\x03", 2) == 0 || strncmp(temp2byte, "\x90\x04", 2) == 0) {
						posizione += 8;
						immagine.seekg(posizione);
						immagine.read(temp4byte, 4);
						if (!bigEndian) flip4(temp4byte);
						posizione = 12 + int4byte(temp4byte);
						immagine.seekg(posizione);
						immagine.read(temp4byte, 4);
						anno = ascii4(temp4byte);
						posizione += 5;
						immagine.seekg(posizione);
						immagine.read(temp2byte, 2);
						mese = ascii2(temp2byte);
						break;
					}
					posizione += 12;
				}
			}
			else if (DateTimeTrovato == false && ExifIFDPointerTrovato == false) {
				cout << "\nnessuna data trovata";
			}
		}
		else
			cout << "non exif!!";
	}
	else
		cout << "impossibile aprire!";
	return 0;
}

void flip4(char* vettore) {
	swap(vettore[0], vettore[3]);
	swap(vettore[1], vettore[2]);
}
void flip2(char* vettore) {
	swap(vettore[0], vettore[1]);
}

int int4byte(char* vettore) {
	int n = int((unsigned char)(vettore[0]) << 24 |
		(unsigned char)(vettore[1]) << 16 |
		(unsigned char)(vettore[2]) << 8 |
		(unsigned char)(vettore[3]));
	return n;
}
int int2byte(char* vettore) {
	int n = (unsigned char)(vettore[0]) << 8 | (unsigned char)(vettore[1]);
	return n;
}

int ascii4(char* vettore) {
	return (((int)vettore[0] - 48) * 1000 + ((int)vettore[1] - 48) * 100 + ((int)vettore[2] - 48) * 10 + ((int)vettore[3] - 48));
}
int ascii2(char* vettore) {
	return (((int)vettore[0] - 48) * 10 + ((int)vettore[1] - 48));
}