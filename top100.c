#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define AAKKOSTON_KOKO (27)

//aakkosto: a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,'

double kaytettyMuistiTrie;//k‰ytet‰‰n lopussa tulostettavissa tiedoissa
double kaytettyMuistiSana;//sama kuin yll‰
int sanastonMaara = 0; //sama kuin yll‰

struct Sana *edeltavaSana; //k‰ytet‰‰n sanojen ketjuttamisessa linkitetyksi listaksi

struct Sana {
    char * sana; 		 //tallennettu sana
    int lkm;            	 //sanojen lukum‰‰r‰
    struct Sana * edeltaja; //viimeksi tallennettu sana (eli edeltavaSana)
};

typedef struct TrieSolmu {
    struct TrieSolmu *lapset[AAKKOSTON_KOKO]; //Solmusta l‰htev‰t kirjaimet (solmut) //Taulukon jokainen solu on yksi aakkoston kirjain (esim. 0 = a, 3 = d ja 26 = ')
    struct Sana *loppu;                         //t‰m‰n avulla voidaan solmu merkit‰ sanan viimeiseksi kirjaimeksi
} TrieSolmu;

int kirjaimenIndeksi(char kirjain);
char indeksinKirjain(int indeksi);
TrieSolmu * uusiSolmu(void);
struct Sana * uusiSana(char *kirjaimet);
void lisaa(TrieSolmu * juuri, char *sana);
void tulostaPuu(TrieSolmu * juuri, char * sana);
void haeTop100(struct Sana * top100[]);
void tarkastaJaLisaa(struct Sana * top100[], struct Sana * tarkastettava);

/*Palauttaa kirjainta vastaavan Trie-solmun taulukon paikan*/
int kirjaimenIndeksi(char kirjain) {
    int k = (int) kirjain;
    if (k == 39) //kirjain '
        return 26;
    if (k >= 97 && k<=122) //pienet kirjaiment
        return k - 97;
    if (k >= 65 && k<= 90)
        return k - 65; //Isot kirjaimet 65-90
    return -1;
}

/*Palauttaa Trie-solmun taulukon paikkaa vastaavan kirjaimen*/
char indeksinKirjain(int indeksi) {
    if (indeksi <= 25) { //kirjaimet a-z
        return (char)97 + indeksi;
    }
    if (indeksi == 26)
        return '\'';
    return '\0';
}

/*Trie-puun uuden solmun luonti*/
TrieSolmu * uusiSolmu(void) {
    TrieSolmu *solmu = NULL;
    solmu = malloc(sizeof(TrieSolmu));
    assert(solmu != NULL);
    int i;
    for (i = 0; i < AAKKOSTON_KOKO; i++)
        solmu->lapset[i] = NULL;
    solmu->loppu = NULL;

    kaytettyMuistiTrie += (long)sizeof(TrieSolmu);

    return solmu;
}

/*Uuden sanan viimeisen kirjaimen merkitsev‰n solmun luonti*/
struct Sana * uusiSana(char *kirjaimet) {
    struct Sana *sana = NULL;
    sana = malloc(sizeof(struct Sana));
    assert(sana != NULL);
    sana->lkm = 1;
    sana->sana = malloc(strlen(kirjaimet)+1);
    assert(sana->sana != NULL);
    strcpy(sana->sana, kirjaimet);

    sana->edeltaja = edeltavaSana; //linkitettyyn listaan lis‰‰minen
    edeltavaSana = sana;

    kaytettyMuistiSana += (long)(sizeof(struct Sana) + strlen(kirjaimet)+1);
    sanastonMaara++;
    return sana;
}

/*Sanan lis‰‰minen Trie-puuhun*/
void lisaa(TrieSolmu * juuri, char *sana) {

    int i;
    int indeksi;
    int pituus = strlen(sana);

    char merkki;
    bool validi = false;
    TrieSolmu * solmuNyt = juuri;

    char tallennettuSana[100]; //Tallennettava sana muodostuu t‰h‰n muuttujaan kirjain kerrallaan. Lopuksi t‰m‰ merkkijono lis‰t‰‰n Sana-solmuun.
    tallennettuSana[0] = '\0';

    int heittomerkki = 0; //T‰m‰n muuttujan avulla voidaan varmistaa, ett‰ sanan viimeinen heittomerkki merkkijonosta saadaan pois.
			  //T‰m‰ joudutaan varmistamaan sen takia, ett‰ ' kuuluu aakkostoon, mutta se ei voi olla merkkijonossa
			  //aakkostoon kuuluvista merkeist‰ viimeinen, kuten tilanteessa ªis'ª. T‰t‰ muuttujaa tarvitaan, koska
			  //viimeisen kirjaimen tarkistaminen ei riit‰, koska silloin esimerkiksi tilanteessa ªis',ª viimeist‰
			  //heittomerkki‰ ei huomattaisi, ja tallennettu merkkijono olisi ªis'ª, vaikka sen pit‰isi olla ªisª.

    for (i = 0; i < pituus; i++) {
        merkki = sana[i];

        if (merkki == '-') { //jos sanasta lˆydet‰‰n v‰liviiva, halkaistaan se kahteen osaan viivan kohdalta
            char toinenSana[strlen(sana)-i];
            int j;
            for (j = 0; j+i+1 < strlen(sana); j++) {
                toinenSana[j] = sana[j+i+1];
            }
            toinenSana[j] = '\0';
            lisaa(juuri, toinenSana);
            break;
        }   else if (validi == false && merkki == '\'') //jos ensimm‰inen aakkostoon kuuluva merkki on ', se j‰tet‰‰n huomiotta
		continue;
	    else if (merkki == '\'') {
		heittomerkki = 1; //Lis‰t‰‰n heittomerkki jonoon, josta se lis‰t‰‰n merkkijonoon, mik‰li aakkostoon kuuluvia merkkej‰ tulee lis‰‰.
				  //Koska jonon pituus on vain yksi, esim "sanat" kuten ªa''aª eiv‰t tallennu oikein, mutta enp‰ viel‰ ole tavannut
				  //sanaa joka sis‰lt‰isi useampia heittomerkkej‰ per‰kk‰in
		continue;
	}

        indeksi = kirjaimenIndeksi(merkki);
        if (indeksi == -1) //jos lˆytyy aakkostoon kuulumattomia merkkej‰, esim. kirjaimia, j‰t‰ ne huomiotta.
            continue;

        int len = strlen(tallennettuSana);
	if (heittomerkki == 1) { //jos on varmistettu, ett‰ lˆydetty '-merkki ei ole viimeinen aakkostoon kuuluva, lis‰t‰‰n se tulokseen
	    tallennettuSana[len] = '\'';
	    tallennettuSana[len+1] = indeksinKirjain(indeksi);
            tallennettuSana[len+2] = '\0';

	    if (solmuNyt->lapset[26] == NULL) {
		solmuNyt->lapset[26] = uusiSolmu(); //lis‰t‰‰n ª'ª puuhun ennen uutta kirjainta
	    }
	    solmuNyt = solmuNyt->lapset[26];
	    heittomerkki = 0;
	} else {
            tallennettuSana[len] = indeksinKirjain(indeksi);
            tallennettuSana[len+1] = '\0';
	}

        validi = true; //sana sis‰lt‰‰ aakkostoon sis‰ltyvi‰ merkkej‰

        if (solmuNyt->lapset[indeksi] == NULL) {
            solmuNyt->lapset[indeksi] = uusiSolmu(); //lis‰‰ uusi kirjain nykyisen solmun lapseksi
        }
        solmuNyt = solmuNyt->lapset[indeksi];
    }
    
    if (validi) { //jos validi sana on lˆytynyt, kasvatetaan sen lukum‰‰r‰‰
        if (solmuNyt->loppu != NULL) {
            solmuNyt->loppu->lkm++;
        } else solmuNyt->loppu = uusiSana(tallennettuSana);
    }
}

/*Tulostaa puun sis‰lt‰m‰t sanat aakkosj‰rjestyksess‰ k‰ytt‰en rekursiota
 (ohjelman toiminnan varmistamista varten)*/
void tulostaPuu(TrieSolmu * juuri, char * sana) {

    TrieSolmu * solmuNyt;
    int i;
    char sanaNyt[100];

    for (i = 0; i < 27; i++) { //k‰y kaikki Trie-solmun mahdolliset lapset l‰pi
        
        strncpy(sanaNyt,sana,100);
        solmuNyt = juuri;
        
        if (juuri->lapset[i] != NULL) {
            int len = strlen(sanaNyt);
            sanaNyt[len] = indeksinKirjain(i); //lis‰t‰‰n lˆydetty kirjain
            sanaNyt[len+1] = '\0';

            solmuNyt = solmuNyt->lapset[i];
            
            if (solmuNyt->loppu != NULL) { //jos kokonainen sana on lˆydetty t‰ss‰ vaiheessa, se tulostetaan
                printf("%s\n", sanaNyt);
            }
            tulostaPuu(solmuNyt, sanaNyt); //k‰yd‰‰n lˆydetty solmu l‰pi (sanaNyt sis‰lt‰‰ t‰h‰n menness‰ lˆydetyt kirjaimet)
        }
    }
}

/*Hakee ja tulostaa 100 yleisint‰ sanaa k‰ytt‰en apuna tarkastaJaLis‰‰-funktiota*/
void haeTop100(struct Sana * top100[]) {

    struct Sana * tarkastettavaSana = edeltavaSana; //edeltavaSana sis‰lt‰‰ viimeisimm‰ksi lis‰tyn Sanan
						    //edeltavaSana on samalla linkitetyn listan p‰‰

    while (tarkastettavaSana!=NULL) { //k‰yd‰‰n koko lista l‰pi
        tarkastaJaLisaa(top100, tarkastettavaSana);	 //lis‰t‰‰n nykyinen Sana top-listaan jos se sinne menee
        tarkastettavaSana = tarkastettavaSana->edeltaja; //edet‰‰n listassa
    }
    printf("100 yleisint‰ sanaa:\n");

    printf("   SANA LKM\n");
    int i;
    for (i = 0; i < 100; i++) {
        if (top100[i]!=NULL)
            printf("%d. %s %d\n", i+1, top100[i]->sana, top100[i]->lkm); //ratkaisun tulostus
    }
}

/*Tarkastaa voidaanko sana lis‰t‰ top100-taulukkoon. Jos voidaan, taulukkoon varataan sanalle paikka ja sana lis‰t‰‰n*/
void tarkastaJaLisaa(struct Sana * top100[], struct Sana * tarkastettava) {

    int i, j;

    for (i = 0; i < 100; i++) {
        if (top100[i] == NULL) {
            top100[i] = tarkastettava;
            break;
        }
        if (top100[i]->lkm > tarkastettava->lkm)
            continue;
        else {
            for (j = 98; j >= i; j--) {
                if (top100[j]!=NULL)
                    top100[j+1] = top100[j];
            }
            top100[i] = tarkastettava;
            break;
        }
    }

}

int main( int argc, char *argv[])
{
    clock_t kokonaisaikaAlku = clock();
    struct Sana *top100[100]; //taulukko 100 yleisimm‰lle sanalle
    int i;
    for (i = 0; i < 100; i++) {
        top100[i] = NULL;
    }

    TrieSolmu * juuriSolmu = uusiSolmu(); //Uusi Trie-puu

    char tiedosto[256]; //tiedoston nimi
    FILE* kirja; //tiedosto
    char sana[256]; //lˆydetty sana

    assert(argc <= 2); //yli 2 komentoriviparametria ei hyv‰ksyt‰

    int sanamaara = 0; //pidet‰‰n yll‰ tiedoston sanojen m‰‰r‰‰
    clock_t lisaysaikaAlku = clock();

    if (argc == 2) { //k‰ytet‰‰n toista parametria tiedoston nimen‰
        printf("Tiedosto: %s\n", argv[1]);

        assert((kirja = fopen(argv[1], "r")) != NULL);

        while (fscanf(kirja, "%s", sana) == 1) {
            lisaa(juuriSolmu, sana); //lis‰t‰‰n lˆydettyj‰ sanoja Trie-puuhun
            sanamaara++;
        }
        fclose(kirja);

    }
    else { //pyydet‰‰n tiedoston nime‰ k‰ytt‰j‰lt‰
        printf("Syˆt‰ tiedoston nimi > ");
        fgets(tiedosto, 256, stdin);

        if((tiedosto[strlen(tiedosto)-1] = '\n')) //poistetaan syˆtteest‰ rivinvaihto
            tiedosto[strlen(tiedosto)-1] = '\0';
        else
            while(getc(stdin) != '\n'); //tarvittaessa tyhjennet‰‰n stdin

        assert((kirja = fopen(tiedosto, "r")) != NULL);

        while (fscanf(kirja, "%s", sana) == 1) {
            lisaa(juuriSolmu, sana); //lis‰t‰‰n lˆydettyj‰ sanoja Trie-puuhun
            sanamaara++;
        }
        fclose(kirja);

    }
    
    clock_t lisaysaikaLoppu = clock();
    
    haeTop100(top100); //tulostaa 100 yleisint‰ sanaa
    
    clock_t kokonaisaikaLoppu = clock();
    
    double kokonaisaika = (double) (kokonaisaikaLoppu - kokonaisaikaAlku) / CLOCKS_PER_SEC;
    double lisaysaika = (double) (lisaysaikaLoppu - lisaysaikaAlku) / CLOCKS_PER_SEC;
    double hakuaika = (double) (kokonaisaikaLoppu - lisaysaikaLoppu) / CLOCKS_PER_SEC;
    
    printf("Sanojen m‰‰r‰ tiedostossa: %d\n", sanamaara);
    printf("Sanaston m‰‰r‰ tiedostossa: %d\n", sanastonMaara);
    printf("Uniikkien sanojen osuus kaikista sanoista: %.1f%%\n", ((double)sanastonMaara/(double)sanamaara) * 100.0);
    
    if (kaytettyMuistiTrie > 1024) {
        if (kaytettyMuistiTrie > 1048576) { //1024^2
            printf("Trie-puuhun k‰ytetty muisti: %.1f MiB\n", kaytettyMuistiTrie / 1048576.0);
            printf("Sana-solmuihin k‰ytetty muisti: %.1f MiB\n", kaytettyMuistiSana / 1048576.0);
            printf("Yhteens‰: %.1f MiB\n", (kaytettyMuistiTrie+kaytettyMuistiSana) / 1048576.0);
        } else {
            printf("Trie-puuhun k‰ytetty muisti: %.1f KiB\n", kaytettyMuistiTrie / 1024.0);
            printf("Sana-solmuihin k‰ytetty muisti: %.1f KiB\n", kaytettyMuistiSana / 1024.0);
            printf("Yhteens‰: %.1f KiB\n", (kaytettyMuistiTrie+kaytettyMuistiSana) / 1024.0);
        }
    }    
    printf("Keskim‰‰r‰inen muistink‰yttˆ per sana: %.1f tavua.\n", (kaytettyMuistiTrie+kaytettyMuistiSana)/sanamaara);
    
    printf("Sanojen lis‰‰miseen kulunut aika: %.f ms.\n", lisaysaika*1000.0);
    printf("Sadan yleisimm‰n sanan hakemiseen kulunut aika: %.f ms.\n", hakuaika*1000.0);
    printf("Ohjelman suoritukseen kulunut aika: %.f ms.\n", kokonaisaika*1000.0);
    return 0;
}
