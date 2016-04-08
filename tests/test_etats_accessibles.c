

	/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux
 *
 *   Copyright (C) 2015 Adrien Boussicault
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
#include "outils.h"
#include "ensemble.h"

void wrap_liberer_automate( Automate * aut ){
	if( aut ){
		liberer_automate(aut);
	}
}

int test_etats_accessibles(){

	int result = 1;

	{
		
		Automate * aut1 = creer_automate();
		ajouter_etat(aut1, 0);
		ajouter_etat(aut1, 1);
		ajouter_etat(aut1, 2);
		ajouter_etat_initial(aut1, 0);
		ajouter_etat_final(aut1,1);
		ajouter_etat_final(aut1,2);
		ajouter_transition(aut1, 0, 'b', 1);
		ajouter_transition(aut1, 1, 'a', 0);
		ajouter_transition(aut1, 1, 'a', 1);
		ajouter_transition(aut1, 1, 'c', 2);
		Ensemble* access_0 = etats_accessibles(aut1, 0);
		Ensemble* access_1 = etats_accessibles(aut1, 1);
		Ensemble* access_2 = etats_accessibles(aut1, 2);
		print_ensemble(access_0,NULL);
		print_ensemble(access_1,NULL);
		print_ensemble(access_2,NULL);
		
		TEST(
			1
			&& aut1
			&& est_dans_l_ensemble(access_0, 1)
			&& est_dans_l_ensemble(access_0, 0)
			&& est_dans_l_ensemble(access_1, 1)
			&& !est_dans_l_ensemble(access_2, 2)
			 
			, result
		);
		wrap_liberer_automate( aut1 );
		liberer_ensemble(access_0);
		liberer_ensemble(access_1);
		liberer_ensemble(access_2);
	}
	

		return result;
}


int main(){

	if( ! test_etats_accessibles() ){ return 1; }

	return 0;
}
