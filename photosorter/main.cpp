#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

using namespace std;
namespace fs = std::filesystem;

void flip4(char*); //inverte vettore
void flip2(char*);
int int4byte(char*); //4 byte big endian -> int
int int2byte(char*);
int ascii4(char*); //carattere ascii 4 byte -> int
int ascii2(char*);

int main() {	
	string sourceDir;
	cout << "Source folder (full path): "; //source directory
	cin >> sourceDir;
	string folder;
	cout << "Destination folder name: ";
	cin >> folder;
	string destDir = sourceDir + folder + "\\"; // destination directory

	bool DateTimeTrovato;
	ifstream immagine;
	string anno, mese;
	
	for (auto& file : fs::recursive_directory_iterator(sourceDir)) {

		if (fs::path(file).extension() == ".jpg" || fs::path(file).extension() == ".JPG") { //controlla se il file è jpg
			immagine.open(file.path(), ios::binary);
			DateTimeTrovato = false;
		
			if (immagine.is_open()) {
	
				int posizione; //offset dall'inizio del file
				int ncampi; //numero dei campi nel IFD corrente
				bool bigEndian = true;
				char temp4byte[4];
				char temp2byte[2];
				bool ExifIFDPointerTrovato = false;

				//controlla se il file è exif
				immagine.seekg(6);
				immagine.read(temp4byte, 4);
				if (strncmp(temp4byte, "Exif", 4) == 0) { 
					//controlla se è big endian
					immagine.seekg(12);
					immagine.read(temp2byte, 2);
					if (strncmp(temp2byte, "II", 2) == 0) { 
						bigEndian = false;
					}
	
					//estrae il numero dei campi
					immagine.seekg(16);
					immagine.read(temp4byte, 4);
					if (!bigEndian) flip4(temp4byte);
					int posizione = 12 + int4byte(temp4byte);
					immagine.seekg(posizione);
					immagine.read(temp2byte, 2);
					if (!bigEndian) flip2(temp2byte);
					ncampi = int2byte(temp2byte);

					//cerca il campo DateTime
					posizione += 2;
					int ExifIFDPointer;
					for (int i = 0; i < ncampi; i++) {
						immagine.seekg(posizione);
						immagine.read(temp2byte, 2);
						if (!bigEndian) flip2(temp2byte);
						if (strncmp(temp2byte, "\x01\x32", 2) == 0) { //se è DateTime estrae anno e mese
							DateTimeTrovato = true;
							posizione += 8;
							immagine.seekg(posizione);
							immagine.read(temp4byte, 4);
							if (!bigEndian) flip4(temp4byte);
							posizione = 12 + int4byte(temp4byte);
							immagine.seekg(posizione);
							immagine.read(temp4byte, 4);
							anno = string(temp4byte, 4);
							posizione += 5;
							immagine.seekg(posizione);
							immagine.read(temp2byte, 2);
							mese = string(temp2byte, 2);
							break;
						}
	
						if (ExifIFDPointerTrovato == false && strncmp(temp2byte, "\x87\x69", 2) == 0) { //controlla se è il puntatore al prossimo IFD
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
					
					//se non è trovato DateTime si cercano DateTimeOriginal e Digitized nel seguente IFD
					if (DateTimeTrovato == false && ExifIFDPointerTrovato == true) {
						//estrae il numero di campi di questo IFD
						posizione = 12 + ExifIFDPointer;
						immagine.seekg(posizione);
						immagine.read(temp2byte, 2);
						if (!bigEndian) flip2(temp2byte);
						ncampi = int2byte(temp2byte);

						//controlla ogni campo per trovare DateTimeOriginal o Digitized
						posizione += 2;	
						for (int i = 0; i < ncampi; i++) {
							immagine.seekg(posizione);
							immagine.read(temp2byte, 2);
							if (!bigEndian) flip2(temp2byte);
							if (strncmp(temp2byte, "\x90\x03", 2) == 0 || strncmp(temp2byte, "\x90\x04", 2) == 0) { //se trovato estrae anno e mese
								DateTimeTrovato = true;
								posizione += 8;
								immagine.seekg(posizione);
								immagine.read(temp4byte, 4);
								if (!bigEndian) flip4(temp4byte);
								posizione = 12 + int4byte(temp4byte);
								immagine.seekg(posizione);
								immagine.read(temp4byte, 4);
								anno = string(temp4byte, 4);
								posizione += 5;
								immagine.seekg(posizione);
								immagine.read(temp2byte, 2);
								mese = string(temp2byte, 2);
								break;
							}
							posizione += 12; //per portarsi all'inizio del prossimo campo
						}
					}
				}
			}
			immagine.close();
	
			//Seconda parte: eventuale creazione della cartella nel formato anno/mese e spostamento del file
			if (DateTimeTrovato && anno != "0000") {
				if (!fs::exists(destDir + (anno + "\\" + mese + "\\"))) {
					fs::create_directories(destDir + (anno + "\\" + mese + "\\"));
				}
				fs::copy(file.path(), destDir + (anno + "\\" + mese + "\\"));						
			}
			else {
				if (!fs::exists(destDir + "senza data")) {
					fs::create_directories(destDir + "senza data");
				}				
				fs::copy(file.path(), destDir + "senza data");								
			}
			fs::remove(file.path()); //rimozione del file originale
		}
	}
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