// Milan Stankovic RA 161/2014 PROJEKAT ORI 
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;


Point sredina(Point2f a, Point2f b) {
	
	Point2f p;
	p.x = (a.x + b.x) / 2;
	p.y = (a.y + b.y) / 2;
	
	return p; 
}

float eukledskaUdaljenost(Point2f P, Point2f Q)
{
	return sqrt(pow(abs(P.x - Q.x), 2) + pow(abs(P.y - Q.y), 2));
}

float udaljenost(Point2f L, Point2f M, Point2f J)
{
	float a, b, c, rez;

	a = -((M.y - L.y) / (M.x - L.x));
	b = 1.0;
	c = (((M.y - L.y) / (M.x - L.x)) * L.x) - L.y;

	rez = (a * J.x + (b * J.y) + c) / sqrt((a * a) + (b * b));
	return rez;
}

float hipotenuza(Point2f L, Point2f M, int& alignement)
{
	float dx, dy;
	dx = M.x - L.x;
	dy = M.y - L.y;

	if (dy != 0)
	{
		alignement = 1;
		return (dy / dx);
	}
	else
	{
		alignement = 0;
		return 0.0;
	}
}


int main() // Fakticki trazenje pravougaonika
{

	VideoCapture kamera(0); // Pokretanje snimanja

	Mat trenutnaSlika;

	kamera >> trenutnaSlika; // frejm u sliku

	Mat siva(trenutnaSlika.size(), CV_MAKETYPE(trenutnaSlika.depth(), 1));	
	Mat ivice(trenutnaSlika.size(), CV_MAKETYPE(trenutnaSlika.depth(), 1));			


	int pronadjenih, A, B, C, gore, desno, dole, m1, m2, prom, provera;
	float AB, BC, CA, duzina, n;

	vector<vector<Point> > konture;
	vector<Vec4i> hijerarhija;
	vector<Point> brojStrana;   
	Point2f tackaA;
	Point2f tackaB;
	Point2f tackaSredina;

	char kraj = 's';

	cout << "Unesite f za izlazak iz programa" << endl;

	while (kraj != 'f')				
	{

		kamera >> trenutnaSlika;						

		cvtColor(trenutnaSlika, siva, CV_RGB2GRAY);		// prebacivanje u grayscale
		Canny(siva, ivice, 100, 200, 3);		// Canny edge za detekciju ivica

		findContours(ivice, konture, hijerarhija, RETR_TREE, CHAIN_APPROX_SIMPLE); // Pronalazak svih kontura

		pronadjenih = 0;  

		vector<Moments> mu(konture.size());
		vector<Point2f> mc(konture.size());

		for (int i = 0; i < konture.size(); i++)
		{
			mu[i] = moments(konture[i], false); // momenti i tezista
			mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
		}


		for (int i = 0; i < konture.size(); i++)
		{
			approxPolyDP(konture[i], brojStrana, arcLength(konture[i], true)*0.02, true);
			if (brojStrana.size() == 4)      // uzimaju se samo oblici sa 4 strane
			{
				int j = i;
				int k = 0;

				while (hijerarhija[j][2] != -1)
				{
					j = hijerarhija[j][2];
					k = k + 1;
				}
				if (hijerarhija[j][2] != -1)
					k = k + 1;

				if (k >= 5) //Dodela kontura 
				{
					if (pronadjenih == 0)		
						A = i;
					else if (pronadjenih == 1)	
						B = i;	
					else if (pronadjenih == 2)	
						C = i;		
					pronadjenih = pronadjenih + 1;
				}
			}
		}


		if (pronadjenih >= 3)		//Pronadjena barem 3 kvadratica
		{

			AB = eukledskaUdaljenost(mc[A], mc[B]);
			BC = eukledskaUdaljenost(mc[B], mc[C]);
			CA = eukledskaUdaljenost(mc[C], mc[A]);

			if (AB > BC && AB > CA) // hipotenuza 
			{
				prom = C; 
				m1 = A; 
				m2 = B;
				tackaA = mc[A];
				tackaB = mc[B];
				
			}
			else if (CA > AB && CA > BC)
			{
				prom = B; 
				m1 = A; 
				m2 = C;
				tackaA = mc[C];
				tackaB = mc[A];
			}
			else if (BC > AB && BC > CA)
			{
				prom = A;  
				m1 = B; 
				m2 = C;
				tackaA = mc[B];
				tackaB = mc[C];
			}

			gore = prom;		// za jednu je svejedno
			dole = m1;
			desno = m2;

			duzina = udaljenost(mc[m1], mc[m2], mc[prom]);	// vertikalna udaljenost 		
			n = hipotenuza(mc[m1], mc[m2], provera);		// hipotenuza

			if (provera == 0) // kako je okrenuto
			{
				dole = m1;
				desno = m2;
			}
			else if (n < 0 && duzina < 0)		
			{
				dole = m1;
				desno = m2;
			}
			else if (n > 0 && duzina < 0)		
			{
				desno = m1;
				dole = m2;
			}
			else if (n < 0 && duzina > 0)				
			{
				desno = m1;
				dole = m2;
			}

			else if (n > 0 && duzina > 0)		
			{
				dole = m1;
				desno = m2;
				
			}



			cv::drawContours(trenutnaSlika, konture, gore, Scalar(255, 0, 0), 2, 8, hijerarhija, 0); // crtanje kontura
			cv::drawContours(trenutnaSlika, konture, desno, Scalar(0, 0, 255), 2, 8, hijerarhija, 0);
			cv::drawContours(trenutnaSlika, konture, dole, Scalar(0, 255, 0), 2, 8, hijerarhija, 0);

			tackaSredina = sredina(tackaA, tackaB); // meni bi zapravo samo ovo trebalo jer bih na ovu tacku stavljao objekat

		}

		imshow("Slika", trenutnaSlika); // prikaz

		kraj = waitKey(1);	//ako se stavi manje ne radi

	}	
	return 0;
}