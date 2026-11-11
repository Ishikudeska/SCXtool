# Mentions légales

> Cette page est une traduction fournie à titre d'information. **La version anglaise fait foi** ; en cas de divergence, le texte anglais (et les fichiers `LICENSE` et `NOTICE` livrés à côté de l'exécutable) prévaut.

## Star Citizen / Cloud Imperium

SCXtool est un **outil communautaire non officiel** pour Star Citizen. Il n'est ni développé, ni approuvé, ni sponsorisé par Cloud Imperium Games (CIG) ou Roberts Space Industries (RSI), et n'y est pas affilié.

**Star Citizen®**, **Roberts Space Industries®** et **Cloud Imperium®** sont des marques déposées de Cloud Imperium Rights LLC et Cloud Imperium Rights Ltd. Toutes les données du jeu Star Citizen, y compris le contenu de `Data.p4k`, sont la propriété intellectuelle de Cloud Imperium Rights LLC.

SCXtool ne redistribue aucun contenu de CIG ou de RSI. Il lit les fichiers de **votre propre installation sous licence de Star Citizen** et réécrit vos textes personnalisés dans cette même installation. Rien du jeu ne quitte votre ordinateur par l'intermédiaire de SCXtool.

## Licences

SCXtool est dérivé de Smart Citizen (licence Apache 2.0, © Osiris DevWorks et contributeurs) et d'unp4k (licence MIT, © dolkensp et contributeurs). Les textes complets des licences et les attributions sont fournis dans les fichiers `LICENSE` et `NOTICE` à côté de l'exécutable.

Bibliothèques tierces :

- **Qt 6** — The Qt Company, utilisée sous **GNU LGPL v3**, liée dynamiquement.
- **zstd** — Meta Platforms, licence **BSD**.
- **zlib** — Jean-loup Gailly et Mark Adler, licence **zlib**.
- **pugixml** — Arseny Kapoulkine, licence **MIT**.

Le logiciel est fourni **« tel quel »**, sans garantie ni condition d'aucune sorte.

## Confidentialité

SCXtool est une application de bureau locale. Il n'envoie ni télémétrie, ni statistiques, ni rapports de plantage, et n'utilise aucun compte.

Il ne se connecte à Internet que pour vérifier sur GitHub si une version plus récente de SCXtool existe (à chaque lancement, et quand vous cliquez sur **Vérifier les mises à jour**) et la télécharger si vous choisissez de mettre à jour, pour télécharger le `global.ini` communautaire d'une langue quand vous passez à cette langue, et pour récupérer un fichier INI depuis une URL quand vous en importez un. La vérification des mises à jour n'envoie rien sur vous ni sur votre PC, à part la version de l'application. Vos modifications, paramètres, sauvegardes et caches restent sur votre ordinateur :

- **Paramètres** — `%APPDATA%\SCXtool\settings.ini` (ou `data\settings.ini` à côté de l'exécutable dans les versions portables).
- **Vos modifications, sauvegardes et cache de textes** — `Documents\SCXtool\<canal>\` par défaut (configurable dans l'onglet Paramètres).
- **Cache DataForge** — `%LOCALAPPDATA%\SCXtool\<canal>\cache\dataforge\`.
- **Journaux et rapports de plantage** — `Documents\SCXtool\logs\`.

## Utilisation à vos risques

SCXtool modifie le texte de localisation, ce que CIG soutient pour les traductions communautaires, ainsi que les options de jeu et les raccourcis que vous choisissez dans l'onglet Game Settings. L'usage que vous en faites relève de votre responsabilité.
