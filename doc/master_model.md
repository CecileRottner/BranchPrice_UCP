Classes "Master Model"

La classe __Master_Model__ (virtuelle) contient les indicateurs communs à tous les problèmes maître, quelle que soit le type de décomposition.
En particulier, on a :
* des infos sur l'instance
* des infos liées aux inégalités Interval up-set générées (nombre total, pointeurs vers les inégalités)
* la solution fractionnaire courante x_frac
* la liste des valeurs duales associées
* le compteur des itérations de pricing

Méthodes virtuelles pures :
- computeFracSol : à partir des variables étendues lambda, calcule la solution en variables d'origine. Dépend de la décomposition choisie.
- discardVar :
- restoreVar :


Classes "Master Variables"

Pour chaque type de décomposition, on a également une classe qui représente les variables étendues générées par la génération de colonnes :
- pour la décomposition par unité/site : classe __Master_Variable__
- pour la décomposition par pas de temps : classe __MasterTime_Variable__
