#include "Wave.h"

void start(int r) {
	Wave wave1("ATrain.wav", r);
	Wave wave2("BeautySlept.wav", r);
	Wave wave3("death2.wav", r);
	Wave wave4("experiencia.wav", r);
	Wave wave5("chanchan.wav", r);
	Wave wave6("female_speech.wav", r);
	Wave wave7("FloorEssence.wav", r);
	Wave wave8("ItCouldBeSweet.wav", r);
	Wave wave9("Layla.wav", r);
	Wave wave10("LifeShatters.wav", r);
	Wave wave11("macabre.wav", r);
	Wave wave12("male_speech.wav", r);
	Wave wave13("SinceAlways.wav", r);
	Wave wave14("thear1.wav", r);
	Wave wave15("TomsDiner.wav", r);
	Wave wave16("velvet.wav", r);

	double tab[16];
	double average = 0;

	tab[0] = wave1.getAverageLsr();
	tab[1] = wave2.getAverageLsr();
	tab[2] = wave3.getAverageLsr();
	tab[3] = wave4.getAverageLsr();
	tab[4] = wave5.getAverageLsr();
	tab[5] = wave6.getAverageLsr();
	tab[6] = wave7.getAverageLsr();
	tab[7] = wave8.getAverageLsr();
	tab[8] = wave9.getAverageLsr();
	tab[9] = wave10.getAverageLsr();
	tab[10] = wave11.getAverageLsr();
	tab[11] = wave12.getAverageLsr();
	tab[12] = wave13.getAverageLsr();
	tab[13] = wave14.getAverageLsr();
	tab[14] = wave15.getAverageLsr();
	tab[15] = wave16.getAverageLsr();

	for (int i = 0; i < 16; i++)
		average += tab[i];

	average /= 16;
	ofstream averageFile("average.txt", ios::app);
	averageFile << r << " " << average << endl;
}

void main() {

	//for (int r = 10; r <= 600; r+=10)
	start(3);
	start(4);
	start(5);
	start(6);
	start(8);
	start(10);
	start(12);
	start(15);
	start(20);
	start(24);
	start(30);
	start(40);
	start(60);
	start(120);
	
	system("pause");
}