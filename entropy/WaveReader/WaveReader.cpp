// WaveReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <cstdio>
#include <cmath>
#include <string>
#include "windows.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

class WaveReader
{
private:
	FOURCC riff; // "RIFF" file descripton header
	DWORD size_of_file; // size of file
	FOURCC wave; // "WAVE" file descripton header
	FOURCC fmt; // "fmt" description header
	DWORD chunk; // size of WAVE section chunck
	WORD pcm; // WAVE type format
	WORD chanel; // mono/stereo
	DWORD sample_rate; // sample rate
	DWORD bytes_per_sec; // bytes/sec
	WORD block_alignment; // Block alignment
	WORD bits_per_sample; // Bits/sample
	FOURCC data; // "data" description header
	DWORD size_of_data; // size of data chunk
	INT16 data_of_file;
	vector<INT16> left;
	vector<double> left_minus;
	vector<INT16> right;
	vector<double> right_minus;
	ofstream new_file;
	FILE *wf;
	double ammount_of_samples;
	const double eps = 1e-12; //wspolczynnik
	vector <double> xDaszekVector;

public:
	WaveReader(string name_of_wave)
	{
		
		const char * c = name_of_wave.c_str();
		wf = fopen(c, "rb");

		if (wf == NULL)
		{
			cout << "File is not opened" << endl;
			exit(-1);
		}

		ReadData();

		ofstream new_file(name_of_wave + ".txt");
		ammount_of_samples = ((size_of_file - 48)-2)/ 2; // liczba probek z obu kanalow
		new_file << name_of_wave << endl;
		new_file << "Liczba pr�bek z obu kana��w: " << fixed << ammount_of_samples << endl;
		normal_vectors(); // wektory wypelniane danymi

		double d = ((first_calculation(ammount_of_samples / 2, left) + first_calculation(ammount_of_samples / 2, right)) / 2);
		new_file << "Przeci�tna energia sygna�u: " << fixed << d << endl; // pierwsze obliczenia
		minus_vectors(); // wektory wypelniane obliczonymi danymi poprzez odjecie wartosci poprzedniej probki od obecnej
		d = ((first_calculation_minus(ammount_of_samples / 2, left_minus) + first_calculation_minus(ammount_of_samples / 2, right_minus)) / 2);
		new_file << "Przeci�tna energia sygna�u po skanowaniu r�nicowym: " << fixed << d << endl;  // drugie obliczenia
		
		new_file << "Entropia: " << ((entro(left) + entro(right)) / 2) << endl; // entro dla normalnych
		new_file << "Entropia z danych po skanowaniu r�nicowym: " << ((entro_minus(left_minus) + entro_minus(right_minus)) / 2) << endl; // entro dla tych po skalowaniu
		SystemOfEquations();
		new_file << "Entropia ze wsp�czynnikiem " << entro_daszek(xDaszekVector)<<endl;


	}
	 
	~WaveReader()
	{
		new_file.close();
	}

	void ReadData() //czytanie danych
	{
		fread(&riff, sizeof(FOURCC), 1, wf);
		fread(&size_of_file, sizeof(DWORD), 1, wf);
		fread(&wave, sizeof(FOURCC), 1, wf);
		fread(&fmt, sizeof(FOURCC), 1, wf);
		fread(&chunk, sizeof(FOURCC), 1, wf);
		fread(&pcm, sizeof(WORD), 1, wf);
		fread(&chanel, sizeof(WORD), 1, wf);
		fread(&sample_rate, sizeof(DWORD), 1, wf);
		fread(&bytes_per_sec, sizeof(DWORD), 1, wf);
		fread(&block_alignment, sizeof(WORD), 1, wf);
		fread(&bits_per_sample, sizeof(WORD), 1, wf);
		fread(&data, sizeof(FOURCC), 1, wf);
		fread(&size_of_data, sizeof(DWORD), 1, wf);
	}

	
	void normal_vectors() // wypelnie wektorow danymi, parzyste do lewego, nieparzyste do prawego
	{
		for (int i = 0; i < ammount_of_samples; i++)
		{
			fread(&data_of_file, sizeof(INT16), 1, wf);
			if (i % 2 == 0)
			{
				left.push_back(data_of_file);
			}
			else
			{
				right.push_back(data_of_file);
			}
		}
	}

	void minus_vectors() // wektory wypelniane obliczonymi danymi poprzez odjecie wartosci poprzedniego sampla od obecnego
	{
		for (int i = 0; i < left.size(); i++)
		{
			if (i == 0)
			{
				left_minus.push_back (left.at(i));
			}
			else
			{
				left_minus.push_back (left.at(i) - left.at(i - 1));
			}
		}
		for (int i = 0; i < right.size(); i++)
		{
			if (i == 0)
			{
				right_minus.push_back(right.at(i));
			}
			else
			{
				right_minus.push_back(right.at(i) - right.at(i - 1));
			}
		}
	}

	double first_calculation(double a, vector<INT16> b) // obliczanie tej sumy
	{
		double full = 0;
		for (INT32 i = 0; i < a; i++)
		{
			full = (double)(full + (((double)b.at(i) * (double)b.at(i))));
		}

		full = full / a;
		
		return full;
	}

	double first_calculation_minus(double a, vector<double> b) // funkcje x_minus to te same funkcje co wyzej tylko przymujace inny typ danych
	{
		double full = 0;
		for (INT32 i = 0; i < a; i++)
		{
			full = (double)(full + (b.at(i) * b.at(i)));
		}

		full = full / a;

		return full;
	}

	double entro(vector<INT16> a) // obliczanie entropi
	{
		double entro = 0;
		vector <double> buffor(131072, 0); // vector przetrzymujacy 2^17 miejsc
		for (int i = 0; i < a.size(); i++)
		{
			buffor.at(a.at(i) + 65536)++; // dodawanie powtarzajacych sie wartosci
		}
		for (int i = 0; i < 131072; i++)
		{
			if (buffor.at(i) != 0) // jezeli probka sie nie pojawila i jest 0 to nie mozemy jej uzyc, bo logarytm wywali nand
			{
				double p_i = (double)buffor.at(i) / a.size();
				entro = entro + (p_i*log2(p_i));
			}
		}
		return entro *(-1);
	}

	double entro_minus(vector<double> a)
	{
		double entro = 0;
		vector <double> buffor(131072, 0);
		for (int i = 0; i < a.size(); i++)
		{
			buffor.at(a.at(i)+ 65536)++;
		}
		for (int i = 0; i < 131072; i++)
		{
			if (buffor.at(i) != 0)
			{
				double p_i = (double)buffor.at(i) / a.size();
				entro = entro + (p_i*log2(p_i));
			}
		}
		return entro *(-1);
	}

	double entro_daszek(vector<double> a)
	{
		double entro = 0;
		vector <double> buffor(16777216, 0);
		for (int i = 0; i < a.size(); i++)
		{
			buffor.at(a.at(i) + 8388608)++;
		}
		for (int i = 0; i < 16777216; i++)
		{
			if (buffor.at(i) != 0)
			{
				double p_i = (double)buffor.at(i) / a.size();
				entro = entro + (p_i*log2(p_i));
			}
		}
		return entro *(-1);
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	// Funkcja dokonuje rozk�adu LU macierzy A
	bool ludist(int n, double ** A)
	{
		int i, j, k;

		for (k = 0; k < n - 1; k++)
		{
			if (fabs(A[k][k]) < eps) return false;

			for (i = k + 1; i < n; i++)
				A[i][k] /= A[k][k];

			for (i = k + 1; i < n; i++)
				for (j = k + 1; j < n; j++)
					A[i][j] -= A[i][k] * A[k][j];
		}

		return true;
	}

	// Funkcja wyznacza wektor X na podstawie A i B
	bool lusolve(int n, double ** A, double * B, double * X)
	{
		int    i, j;
		double s;

		X[0] = B[0];

		for (i = 1; i < n; i++)
		{
			s = 0;

			for (j = 0; j < i; j++) s += A[i][j] * X[j];

			X[i] = B[i] - s;
		}

		if (fabs(A[n - 1][n - 1]) < eps) return false;

		X[n - 1] /= A[n - 1][n - 1];

		for (i = n - 2; i >= 0; i--)
		{
			s = 0;

			for (j = i + 1; j < n; j++) s += A[i][j] * X[j];

			if (fabs(A[i][i]) < eps) return false;

			X[i] = (X[i] - s) / A[i][i];
		}

		return true;
	}

	
	void SystemOfEquations() {
		double **A, *B, *X;
		int n, r, i, j;
		
		r = 3;
		n = r;

		cout << setprecision(4) << fixed;

		// tworzymy macierze A, B i X
		A = new double *[n];
		B = new double[n];
		X = new double[n];

		for (i = 0; i < n; i++) 
			A[i] = new double[n];

		int N = ammount_of_samples/2; //dla jednego kanalu
		j = 1;
		double sum = 0; //zerowanie sumy
		double sum2 = 0;
		vector<double>a; //wektor dla macierzy X
		vector<double>b; //wektor dla macierzy P

		for (i = 1; i <= r; i++)
		{
			for (int z = r; z < N; z++)
			{
				sum += (left.at(z - i)*left.at(z - j)) + (right.at(z - i) * right.at(z - j)); //suma dla element�w macierzy X
				sum2 += (left.at(z) * left.at(z - i)) + (right.at(z) * right.at(z - i)); //suma dla element�w macierzy P
			}
			a.push_back(sum);

			if (j == 1)
			{
				b.push_back(sum2);
			}
			sum = 0;
			sum2 = 0;
			if (i == r && j != r)
			{
				j++;
				i = 0;
			}
		}

		int licznik=0;
		j = 0;
		for (i = 0; i < n; i++){
			
			A[i][j] = a.at(licznik); // Macierz X (3x3)
			licznik++;
			B[i] = b.at(i); // Macierz P (3x1)
		
			if (i == n-1 && j != n-1)
			{
				i = -1;
				j++;
			}
		}

		//wyswietlamy:
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++)
				cout << A[i][j] << "\t";
			cout << "\t" << B[i] << endl;
		}

		// rozwi�zujemy uk�ad i wy�wietlamy wyniki
		double sumA = 0;
		if (ludist(n, A) && lusolve(n, A, B, X))
		{
			for (i = 0; i < n; i++) {
				cout << "a" << i + 1 << " = " << setw(9) << X[i] << endl;
				sumA += X[i];
			}
			cout << "Suma wspolczynnikow: " << sumA << endl;
		}
		else cout << "DZIELNIK ZERO\n";

		vector<double>maleA;
		for (int i = 0; i < r; i++) {
			maleA.push_back(X[i]);
		}

		double xDaszek=0;
		for (size_t i = 0; i < ammount_of_samples/2; i++)
		{
			if (i <= r)
			{
				xDaszekVector.push_back(right.at(i));
			}
			else {
				for (size_t j = 0; j < r; j++)
				{
					xDaszek += maleA.at(j) * right.at(i-j);
				}
				xDaszekVector.push_back(floor(xDaszek + 0.5));
			}
		}
		// usuwamy macierze z pami�ci
		for (i = 0; i < n; i++) delete[] A[i];
		delete[] A;
		delete[] B;
		delete[] X;
	}

};

void main()
{
	WaveReader wave1("ATrain.wav");
	/*WaveReader wave2("BeautySlept.wav");
	WaveReader wave3("death2.wav");
	WaveReader wave4("experiencia.wav");
	WaveReader wave5("female_speech.wav");
	WaveReader wave6("FloorEssence.wav");
	WaveReader wave7("ItCouldBeSweet.wav");
	WaveReader wave8("Layla.wav");
	WaveReader wave9("LifeShatters.wav");
	WaveReader wave10("macabre.wav");
	WaveReader wave11("male_speech.wav");
	WaveReader wave12("SinceAlways.wav");
	WaveReader wave13("thear1.wav");
	WaveReader wave14("TomsDiner.wav");
	WaveReader wave15("velvet.wav");
	WaveReader wave16("chanchan.wav");*/

	system("pause");
}