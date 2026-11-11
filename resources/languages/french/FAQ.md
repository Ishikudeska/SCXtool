# Foire aux questions

Des réponses rapides aux questions les plus fréquentes. Si votre question n'est pas ici, cliquez sur le lien **Retours** en bas de la fenêtre et posez-la sur notre Discord.

## Comment annuler les modifications faites par SCXtool ?

Facilement, et à tout moment. SCXtool ne modifie jamais les fichiers originaux du jeu sur place, donc revenir à la version vanilla ne prend qu'un clic :

- **Barre d'outils → Plus → Effacer la localisation** supprime le `global.ini` personnalisé écrit par SCXtool. Le jeu revient immédiatement à son texte intégré. Vos modifications ne sont pas perdues, elles restent enregistrées dans l'application et vous pouvez les réappliquer quand vous le souhaitez.
- Vous préférez revenir en arrière d'une seule version plutôt que de tout effacer ? **Barre d'outils → Plus → Restaurer une sauvegarde** ramène le fichier du jeu à une sauvegarde horodatée (SCXtool conserve les 5 dernières, et en crée une nouvelle à chaque application).
- Vous avez modifié quelque chose dans l'onglet **Game Settings** ? Cliquez sur **Backups** dans cet onglet pour restaurer un `attributes.xml` ou un `actionmaps.xml` antérieur. SCXtool en fait une copie avant chaque enregistrement et conserve les 5 dernières de chaque fichier.

Vos modifications personnelles se trouvent dans `user.ini`, dans votre dossier de données SCXtool, séparé du jeu, donc effacer le fichier du jeu ne les touche jamais.

## Vais-je être banni pour avoir utilisé SCXtool ?

SCXtool modifie le texte de localisation (les mots affichés par le jeu) et, depuis l'onglet Game Settings, vos options de jeu et vos raccourcis. Il ne touche pas à la logique du jeu, ne vous donne aucun avantage et ne communique pas avec les serveurs de CIG. Nos modifications **devraient** être sans risque.

CIG soutient publiquement la localisation communautaire. Leur billet [Community Localization Update](https://robertsspaceindustries.com/spectrum/community/SC/forum/1/thread/star-citizen-community-localization-update) présente leur soutien officiel aux traductions faites par les joueurs, ce que nous comprenons comme autorisant explicitement le type de modification de localisation que fait SCXtool.

Des streamers très en vue mènent des projets de localisation similaires au grand jour, et aucun d'eux ne s'est vu demander d'arrêter.

Cela dit : la façon dont vous utilisez SCXtool est à vos propres risques. Nos modifications devraient être sans danger, mais pour tout ce que vous faites vous-même, vous et vos associés êtes responsables des dommages éventuels. En cas de doute sur la pertinence d'une modification, restez sur du cosmétique et gardez une sauvegarde.

## Quels fichiers SCXtool modifie-t-il ?

Seulement ceux-ci, et uniquement quand vous le demandez :

- `StarCitizen\<canal>\data\Localization\<langue>\global.ini` — le fichier de localisation du jeu pour le canal (LIVE, PTU, etc.) et la langue que vous avez sélectionnés, écrit lorsque vous cliquez sur **Appliquer les enrichissements**. SCXtool sauvegarde d'abord le fichier existant, puis écrit le résultat fusionné.
- `g_language` dans votre `user.cfg`, défini en même temps pour que le jeu charge la bonne localisation.
- `attributes.xml` (options de jeu) et `actionmaps.xml` (raccourcis et options des manettes) dans `StarCitizen\<canal>\user\client\0\Profiles\default\`, écrits uniquement lorsque vous cliquez sur **Save to Game** dans l'onglet **Game Settings** ou restaurez l'une de ses sauvegardes. SCXtool sauvegarde d'abord chaque fichier et n'écrit rien tant que le jeu est lancé.

Rien d'autre dans votre installation du jeu n'est touché.

Tout ce que SCXtool génère pour son propre usage (le cache source, les fichiers d'enrichissements, les sauvegardes, votre `user.ini`) se trouve dans votre dossier de données SCXtool, pas dans le jeu.

## Pourquoi Windows dit-il que cette application n'est pas reconnue ?

Parce que SCXtool n'est pas encore signé numériquement. Windows SmartScreen et Smart App Control signalent toute nouvelle application d'un éditeur pour lequel ils n'ont pas de certificat de signature enregistré, même si elle est totalement sûre. C'est un avertissement du type « nous ne connaissons pas ceci », pas « ceci est dangereux ».

Pour l'exécuter : sur l'invite SmartScreen, cliquez sur **Plus d'infos → Exécuter quand même**. Si Smart App Control le bloque complètement, vous pouvez autoriser l'application depuis son invite, ou désactiver temporairement Smart App Control, installer, puis le réactiver.

La signature de code est sur notre feuille de route, ce qui fera disparaître cet avertissement. En attendant, ne téléchargez SCXtool que depuis nos versions officielles sur GitHub, pour être sûr d'avoir la version authentique.
