/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux
 *
 *   Copyright (C) 2014, 2015 Adrien Boussicault
 *
 *    This Library is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This Library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this Library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "automate.h"
#include "table.h"
#include "ensemble.h"
#include "outils.h"
#include "fifo.h"

#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> 

#include <assert.h>

#include <math.h>



void action_get_max_etat( const intptr_t element, void* data ){
	int * max = (int*) data;
	if( *max < element ) *max = element;
}

int get_max_etat( const Automate* automate ){
	int max = INT_MIN;

	pour_tout_element( automate->etats, action_get_max_etat, &max );

	return max;
}

void action_get_min_etat( const intptr_t element, void* data ){
	int * min = (int*) data;
	if( *min > element ) *min = element;
}


int get_min_etat( const Automate* automate ){
	int min = INT_MAX;

	pour_tout_element( automate->etats, action_get_min_etat, &min );

	return min;
}


int comparer_cle(const Cle *a, const Cle *b) {
	if( a->origine < b->origine )
		return -1;
	if( a->origine > b->origine )
		return 1;
	if( a->lettre < b->lettre )
		return -1;
	if( a->lettre > b->lettre )
		return 1;
	return 0;
}

void print_cle( const Cle * a){
	printf( "(%d, %c)" , a->origine, (char) (a->lettre) );
}

void supprimer_cle( Cle* cle ){
	xfree( cle );
}

void initialiser_cle( Cle* cle, int origine, char lettre ){
	cle->origine = origine;
	cle->lettre = (int) lettre;
}

Cle * creer_cle( int origine, char lettre ){
	Cle * result = xmalloc( sizeof(Cle) );
	initialiser_cle( result, origine, lettre );
	return result;
}

Cle * copier_cle( const Cle* cle ){
	return creer_cle( cle->origine, cle->lettre );
}

Automate * creer_automate(){
	Automate * automate = xmalloc( sizeof(Automate) );
	automate->etats = creer_ensemble( NULL, NULL, NULL );
	automate->alphabet = creer_ensemble( NULL, NULL, NULL );
	automate->transitions = creer_table(
		( int(*)(const intptr_t, const intptr_t) ) comparer_cle , 
		( intptr_t (*)( const intptr_t ) ) copier_cle,
		( void(*)(intptr_t) ) supprimer_cle
	);
	automate->initiaux = creer_ensemble( NULL, NULL, NULL );
	automate->finaux = creer_ensemble( NULL, NULL, NULL );
	automate->vide = creer_ensemble( NULL, NULL, NULL ); 
	return automate;
}

Automate * translater_automate_entier( const Automate* automate, int translation ){
	Automate * res = creer_automate();

	Ensemble_iterateur it;
	for( 
		it = premier_iterateur_ensemble( get_etats( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat( res, get_element( it ) + translation );
	}

	for( 
		it = premier_iterateur_ensemble( get_initiaux( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat_initial( res, get_element( it ) + translation );
	}

	for( 
		it = premier_iterateur_ensemble( get_finaux( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat_final( res, get_element( it ) + translation );
	}

	// On ajoute les lettres
	for(
		it = premier_iterateur_ensemble( get_alphabet( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_lettre( res, (char) get_element( it ) );
	}

	Table_iterateur it1;
	Ensemble_iterateur it2;
	for(
		it1 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it1 );
		it1 = iterateur_suivant_table( it1 )
	){
		Cle * cle = (Cle*) get_cle( it1 );
		Ensemble * fins = (Ensemble*) get_valeur( it1 );
		for(
			it2 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it2 );
			it2 = iterateur_suivant_ensemble( it2 )
		){
			int fin = get_element( it2 );
			ajouter_transition(
				res, cle->origine + translation, cle->lettre, fin + translation
			);
		}
	};

	return res;
}


void liberer_automate( Automate * automate ){
	assert( automate );
	liberer_ensemble( automate->vide );
	liberer_ensemble( automate->finaux );
	liberer_ensemble( automate->initiaux );
	pour_toute_valeur_table(
		automate->transitions, ( void(*)(intptr_t) ) liberer_ensemble
	);
	liberer_table( automate->transitions );
	liberer_ensemble( automate->alphabet );
	liberer_ensemble( automate->etats );
	xfree(automate);
}

const Ensemble * get_etats( const Automate* automate ){
	return automate->etats;
}

const Ensemble * get_initiaux( const Automate* automate ){
	return automate->initiaux;
}

const Ensemble * get_finaux( const Automate* automate ){
	return automate->finaux;
}

const Ensemble * get_alphabet( const Automate* automate ){
	return automate->alphabet;
}

void ajouter_etat( Automate * automate, int etat ){
	ajouter_element( automate->etats, etat );
}

void ajouter_lettre( Automate * automate, char lettre ){
	ajouter_element( automate->alphabet, lettre );
}

void ajouter_transition(
	Automate * automate, int origine, char lettre, int fin
){
	ajouter_etat( automate, origine );
	ajouter_etat( automate, fin );
	ajouter_lettre( automate, lettre );

	Cle cle;
	initialiser_cle( &cle, origine, lettre );
	Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
	Ensemble * ens;
	if( iterateur_est_vide( it ) ){
		ens = creer_ensemble( NULL, NULL, NULL );
		add_table( automate->transitions, (intptr_t) &cle, (intptr_t) ens );
	}else{
		ens = (Ensemble*) get_valeur( it );
	}
	ajouter_element( ens, fin );
}

void ajouter_etat_final(
	Automate * automate, int etat_final
){
	ajouter_etat( automate, etat_final );
	ajouter_element( automate->finaux, etat_final );
}

void ajouter_etat_initial(
	Automate * automate, int etat_initial
){
	ajouter_etat( automate, etat_initial );
	ajouter_element( automate->initiaux, etat_initial );
}

const Ensemble * voisins( const Automate* automate, int origine, char lettre ){
	Cle cle;
	initialiser_cle( &cle, origine, lettre );
	Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
	if( ! iterateur_est_vide( it ) ){
		return (Ensemble*) get_valeur( it );
	}else{
		return automate->vide;
	}
}

Ensemble * delta1(
	const Automate* automate, int origine, char lettre
){
	Ensemble * res = creer_ensemble( NULL, NULL, NULL );
	ajouter_elements( res, voisins( automate, origine, lettre ) );
	return res; 
}

Ensemble * delta(
	const Automate* automate, const Ensemble * etats_courants, char lettre
){
	Ensemble * res = creer_ensemble( NULL, NULL, NULL );

	Ensemble_iterateur it;
	for( 
		it = premier_iterateur_ensemble( etats_courants );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		const Ensemble * fins = voisins(
			automate, get_element( it ), lettre
		);
		ajouter_elements( res, fins );
	}

	return res;
}

Ensemble * delta_star(
	const Automate* automate, const Ensemble * etats_courants, const char* mot
){
	int len = strlen( mot );
	int i;
	Ensemble * old = copier_ensemble( etats_courants );
	Ensemble * new = old;
	for( i=0; i<len; i++ ){
		new = delta( automate, old, *(mot+i) );
		liberer_ensemble( old );
		old = new;
	}
	return new;
}

void pour_toute_transition(
	const Automate* automate,
	void (* action )( int origine, char lettre, int fin, void* data ),
	void* data
){
	Table_iterateur it1;
	Ensemble_iterateur it2;
	for(
		it1 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it1 );
		it1 = iterateur_suivant_table( it1 )
	){
		Cle * cle = (Cle*) get_cle( it1 );
		Ensemble * fins = (Ensemble*) get_valeur( it1 );
		for(
			it2 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it2 );
			it2 = iterateur_suivant_ensemble( it2 )
		){
			int fin = get_element( it2 );
			action( cle->origine, cle->lettre, fin, data );
		}
	};
}

Automate* copier_automate( const Automate* automate ){
	Automate * res = creer_automate();
	Ensemble_iterateur it1;
	// On ajoute les états de l'automate
	for(
		it1 = premier_iterateur_ensemble( get_etats( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat( res, get_element( it1 ) );
	}
	// On ajoute les états initiaux
	for(
		it1 = premier_iterateur_ensemble( get_initiaux( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat_initial( res, get_element( it1 ) );
	}
	// On ajoute les états finaux
	for(
		it1 = premier_iterateur_ensemble( get_finaux( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat_final( res, get_element( it1 ) );
	}
	// On ajoute les lettres
	for(
		it1 = premier_iterateur_ensemble( get_alphabet( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_lettre( res, (char) get_element( it1 ) );
	}
	// On ajoute les transitions
	Table_iterateur it2;
	for(
		it2 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it2 );
		it2 = iterateur_suivant_table( it2 )
	){
		Cle * cle = (Cle*) get_cle( it2 );
		Ensemble * fins = (Ensemble*) get_valeur( it2 );
		for(
			it1 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it1 );
			it1 = iterateur_suivant_ensemble( it1 )
		){
			int fin = get_element( it1 );
			ajouter_transition( res, cle->origine, cle->lettre, fin );
		}
	}
	return res;
}

Automate * translater_automate(
	const Automate * automate, const Automate * automate_a_eviter
){
	if(
		taille_ensemble( get_etats(automate) ) == 0 ||
		taille_ensemble( get_etats(automate_a_eviter) ) == 0
	){
		return copier_automate( automate );
	}
	
	int translation = 
		get_max_etat( automate_a_eviter ) - get_min_etat( automate ) + 1; 

	return translater_automate_entier( automate, translation );
	
}

int est_une_transition_de_l_automate(
	const Automate* automate,
	int origine, char lettre, int fin
){
	return est_dans_l_ensemble( voisins( automate, origine, lettre ), fin );
}

int est_un_etat_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_etats( automate ), etat );
}

int est_un_etat_initial_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_initiaux( automate ), etat );
}

int est_un_etat_final_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_finaux( automate ), etat );
}

int est_une_lettre_de_l_automate( const Automate* automate, char lettre ){
	return est_dans_l_ensemble( get_alphabet( automate ), lettre );
}

void print_ensemble_2( const intptr_t ens ){
	print_ensemble( (Ensemble*) ens, NULL );
}

void print_lettre( intptr_t c ){
	printf("%c", (char) c );
}

void print_automate( const Automate * automate ){
	printf("- Etats : ");
	print_ensemble( get_etats( automate ), NULL );
	printf("\n- Initiaux : ");
	print_ensemble( get_initiaux( automate ), NULL );
	printf("\n- Finaux : ");
	print_ensemble( get_finaux( automate ), NULL );
	printf("\n- Alphabet : ");
	print_ensemble( get_alphabet( automate ), print_lettre );
	printf("\n- Transitions : ");
	print_table( 
		automate->transitions,
		( void (*)( const intptr_t ) ) print_cle, 
		( void (*)( const intptr_t ) ) print_ensemble_2,
		""
	);
	printf("\n");
}

int le_mot_est_reconnu( const Automate* automate, const char* mot ){
	Ensemble * arrivee = delta_star( automate, get_initiaux(automate) , mot ); 
	
	int result = 0;

	Ensemble_iterateur it;
	for(
		it = premier_iterateur_ensemble( arrivee );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		if( est_un_etat_final_de_l_automate( automate, get_element(it) ) ){
			result = 1;
			break;
		}
	}
	liberer_ensemble( arrivee );
	return result;
}

Automate * mot_to_automate( const char * mot ){
	Automate * automate = creer_automate();
	int i = 0;
	int size = strlen( mot );
	for( i=0; i < size; i++ ){
		ajouter_transition( automate, i, mot[i], i+1 );
	}
	ajouter_etat_initial( automate, 0 );
	ajouter_etat_final( automate, size );
	return automate;
}

// Ajout de transitions dans l'automate de l'union
void ajout_transition_dans_union(int origine, char lettre, int fin, void* data ){
	
	ajouter_transition(data, origine, lettre, fin);
	
}

Automate * creer_union_des_automates(
	const Automate * automate_1, const Automate * automate_2
){ 
	
	// Copie de A1 dans union, et translation de A2 pour éviter conflits d'états
	Automate * auto_union = copier_automate(automate_1);
	Automate * tmp = translater_automate(automate_2, auto_union);
	
	// ajout des états de A2 dans union
	Ensemble_iterateur it;
	for( it = premier_iterateur_ensemble (get_etats(tmp)) ;
		 ! iterateur_ensemble_est_vide(it);
		 it = iterateur_suivant_ensemble (it)) 
	{
		ajouter_etat(auto_union, get_element(it));
			
		if (est_un_etat_initial_de_l_automate(tmp,get_element(it)))
			ajouter_etat_initial(auto_union,get_element(it));
	
		if (est_un_etat_final_de_l_automate(tmp,get_element(it)))
			ajouter_etat_final(auto_union,get_element(it));
				
	}
	
	// ajout des transitions de A2 dans union
	pour_toute_transition(tmp, ajout_transition_dans_union, auto_union);
	
	return auto_union;	
}

Ensemble* etats_accessibles( const Automate * automate, int etat ){
	Ensemble* access = creer_ensemble(NULL,NULL,NULL);
	const Ensemble* alphabet = get_alphabet(automate);
	
	Ensemble_iterateur it_access = premier_iterateur_ensemble(access);
	Ensemble_iterateur it_alphabet;
	
	// on ajoute dans access les états accessibles directement depuis etat
	for (it_alphabet = premier_iterateur_ensemble(alphabet);
	! iterateur_ensemble_est_vide(it_alphabet);
	it_alphabet = iterateur_suivant_ensemble(it_alphabet))
	{
		ajouter_elements(access, delta1(automate, etat, get_element(it_alphabet)));
	} 
	
	// pour chaque état de access, on ajoute le delta1 de cet état pour toute lettre de l'alphabet
	for (it_access = premier_iterateur_ensemble(access) ;
	!iterateur_ensemble_est_vide(it_access);
	it_access = iterateur_suivant_ensemble(it_access))
	{
		
		for (it_alphabet = premier_iterateur_ensemble(alphabet);
			! iterateur_ensemble_est_vide(it_alphabet);
			it_alphabet = iterateur_suivant_ensemble(it_alphabet))
		{
			
			ajouter_elements(access, 
							delta1(automate,
								get_element(it_access),
								get_element(it_alphabet)));
		} 
	} 
	
	return access;
}

Ensemble* accessibles( const Automate * automate ){
	// pour chaque état initial
	// etat_accessible (aut, etat_i)
	Ensemble* access = creer_ensemble(NULL,NULL,NULL);
	const Ensemble* initiaux = get_initiaux(automate);
	Ensemble_iterateur it_access;
	
	// puis on les ajoute dans access
	for (it_access = premier_iterateur_ensemble(initiaux) ;
	!iterateur_ensemble_est_vide(it_access);
	it_access = iterateur_suivant_ensemble(it_access))
	{
		ajouter_elements(access, etats_accessibles(automate ,get_element(it_access)));
	}
	
	return access;
}

void ajout_transition_accessible(int origine, char lettre, int fin, void* data) {

	if (est_un_etat_de_l_automate(data, origine))
	{
		ajouter_transition(data, origine, lettre, fin);
	}
} 

Automate *automate_accessible( const Automate * automate ){
	Automate * res = creer_automate();
	Ensemble * access = accessibles(automate);
	Ensemble_iterateur it_access;
	
	//recopier les états accessibles dans res
	for(it_access = premier_iterateur_ensemble(access);
		!  iterateur_ensemble_est_vide(it_access);
		it_access = iterateur_suivant_ensemble(it_access)
		)
	{
		ajouter_etat(res, get_element(it_access));
		// initiaux accessibles
		if (est_un_etat_initial_de_l_automate(automate, get_element(it_access)))
		{
			ajouter_etat_initial(res, get_element(it_access));
		}
		// finaux accessibles
		if (est_un_etat_final_de_l_automate(automate, get_element(it_access)))
		{
			ajouter_etat_final(res, get_element(it_access));
		}		
	}
	// chaque transition dont l'origine est dans res doit être ajoutée car elle est accessible.
	pour_toute_transition(automate, ajout_transition_accessible, res);
		
	return res;	
}

void ajout_transition_miroir(int origine, char lettre, int fin, void* data) {
	ajouter_transition(data, fin, lettre, origine);
} 

Automate * miroir( const Automate * automate){

	Automate * res = creer_automate();
	
	Ensemble_iterateur it_miroir;
	
	//recopier les états 
	for(it_miroir = premier_iterateur_ensemble(get_etats(automate));
		!  iterateur_ensemble_est_vide(it_miroir);
		it_miroir = iterateur_suivant_ensemble(it_miroir)
		)
	{
		ajouter_etat(res, get_element(it_miroir));
		// placer initiaux de automate dans les finaux de res
		if (est_un_etat_initial_de_l_automate(automate, get_element(it_miroir)))
		{
			ajouter_etat_final(res, get_element(it_miroir));
		}
		// placer finaux de automate dans les initiaux de res
		if (est_un_etat_final_de_l_automate(automate, get_element(it_miroir)))
		{
			ajouter_etat_initial(res, get_element(it_miroir));
		}		
	}
	
	pour_toute_transition(automate, ajout_transition_miroir, res);
	return res;
}

Automate * creer_automate_du_melange(
	const Automate* automate_1,  const Automate* automate_2
){
	Automate * res = creer_automate();
	// on récupère les états de nos deux automates pour en faire des couples
	Ensemble_iterateur it_a1, it_a2; 
	typedef struct { int x; int y; } Couple;   // On définit le couple
	Table * t = creer_table( NULL, NULL, NULL );
	int cpt=0;
	
	for(it_a1 = premier_iterateur_ensemble(get_etats(automate_1)); // 
		! iterateur_ensemble_est_vide(it_a1);
		it_a1 = iterateur_suivant_ensemble(it_a1))
	{
		for(it_a2 = premier_iterateur_ensemble(get_etats(automate_2));
		! iterateur_ensemble_est_vide(it_a2);
		it_a2 = iterateur_suivant_ensemble(it_a2))
		{
			Couple * c = malloc( sizeof(Couple) );       // On crée une valeur
			c->x = get_element(it_a1);					// x sera l'état de l'automate 1  }
			c->y = get_element(it_a2);					// y sera l'état de l'automate 2  }		=> état xy
			add_table(t, cpt, (intptr_t) c);
			ajouter_etat(res,cpt);
			
			// on considère un couple comme état initial de notre automate que si x et y sont des états initiaux de leurs automates respectifs.
			if (est_un_etat_initial_de_l_automate(automate_1, c->x) && est_un_etat_initial_de_l_automate(automate_2 , c->y))
			{
				ajouter_etat_initial(res,cpt);
			}
			if (est_un_etat_final_de_l_automate(automate_1, c->x) && est_un_etat_final_de_l_automate(automate_2 , c->y))
			{
				ajouter_etat_final(res,cpt);
			}
			cpt++;
		}	
	}
	// soient deux couples c1 et c2.
	// si c1.x -> c2.x existe et que le y de c1 est égal à celui de c2 on ajoute la transition 
	// idem pour y si les x ne diffèrent pas
	
	Ensemble * al = creer_union_ensemble(get_alphabet(automate_1),get_alphabet(automate_2));
	Table_iterateur it_table , it_table2;
	Ensemble_iterateur it_al, it_dx, it_dy;
	
	for(it_table = premier_iterateur_table(t) ; // on va parcourir la table de nos couples pour ajouter les transitions à nos automates
		! iterateur_est_vide(it_table);
		it_table = iterateur_suivant_table(it_table))
	{
		for(it_al = premier_iterateur_ensemble(al); 
		! iterateur_ensemble_est_vide(it_al);
		it_al = iterateur_suivant_ensemble(it_al))
		{
			Ensemble * dx = delta1(automate_1, ((Couple*)get_valeur(it_table))->x, get_element(it_al));
			Ensemble * dy = delta1(automate_2, ((Couple*)get_valeur(it_table))->y, get_element(it_al));
			
			for(it_dx = premier_iterateur_ensemble(dx);
			! iterateur_ensemble_est_vide(it_dx);
			it_dx = iterateur_suivant_ensemble(it_dx))
			{

				for(it_table2 = premier_iterateur_table(t) ;
				! iterateur_est_vide(it_table2);
				it_table2 = iterateur_suivant_table(it_table2))
				{
					if (((Couple*)get_valeur(it_table2))->x == get_element(it_dx)  // si C->x a changé
					&&  ((Couple*)get_valeur(it_table2))->y == ((Couple*)get_valeur(it_table))->y ) // mais pas C->y
					{
						ajouter_transition(res, get_cle(it_table), get_element(it_al), get_cle(it_table2)); // on ajoute la transition
					}
					
				}
			}
			for(it_dy = premier_iterateur_ensemble(dy);
			! iterateur_ensemble_est_vide(it_dy);
			it_dy = iterateur_suivant_ensemble(it_dy))
			{
				for(it_table2 = premier_iterateur_table(t) ;
				! iterateur_est_vide(it_table2);
				it_table2 = iterateur_suivant_table(it_table2))
				{
					if (((Couple*)get_valeur(it_table2))->y == get_element(it_dy)  // si C->y a changé
					&&  ((Couple*)get_valeur(it_table2))->x == ((Couple*)get_valeur(it_table))->x ) // mais pas C->x
					{
						ajouter_transition(res, get_cle(it_table), get_element(it_al), get_cle(it_table2));
					}
				}	
			}
			liberer_ensemble(dx);
			liberer_ensemble(dy);
		}
	}
	 
	 return res;
}

Automate * creer_automate_deterministe( const Automate* automate ){
	A_FAIRE_RETURN( NULL );
}

