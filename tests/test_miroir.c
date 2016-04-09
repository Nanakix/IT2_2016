



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

int test_miroir(){

	int result = 1;

	{
		
		const Automate * aut1 = mot_to_automate("abab");
		Automate * mirror = miroir(aut1);
		
		TEST(
			1
			&& aut1
			&& le_mot_est_reconnu(mirror, "baba")
			&& !le_mot_est_reconnu(mirror, "abab")
			
			, result
		);
		
		//??wrap_liberer_automate( aut1 );
		wrap_liberer_automate( mirror );
		
	}
	

		return result;
}


int main(){

	if( ! test_miroir() ){ return 1; }

	return 0;
}
