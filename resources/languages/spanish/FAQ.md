# Preguntas frecuentes

Respuestas rápidas a lo que la gente más pregunta. Si tu pregunta no está aquí, pulsa el enlace **Comentarios** en la parte inferior y pregúntanos en Discord.

## ¿Cómo deshago los cambios que hace SCXtool?

Fácilmente, y en cualquier momento. SCXtool nunca edita los archivos originales del juego directamente, así que volver a la versión vanilla es cuestión de un clic:

- **Barra de herramientas → Más → Limpiar localización** borra el `global.ini` personalizado que escribió SCXtool. El juego vuelve de inmediato a su texto integrado. Tus ediciones no se pierden, siguen guardadas en la aplicación y puedes volver a aplicarlas cuando quieras.
- ¿Prefieres retroceder una sola versión en lugar de borrarlo todo? **Barra de herramientas → Más → Restaurar copia** devuelve el archivo del juego a una copia de seguridad con fecha y hora (SCXtool conserva las últimas 5, y crea una nueva cada vez que aplicas).
- ¿Has cambiado algo en la pestaña **Game Settings**? Pulsa **Backups** en esa pestaña para restaurar un `attributes.xml` o `actionmaps.xml` anterior. SCXtool hace una copia antes de cada guardado y conserva las últimas 5 de cada archivo.

Tus ediciones personales viven en `user.ini`, en tu carpeta de datos de SCXtool, separada del juego, así que limpiar el archivo del juego nunca las afecta.

## ¿Me banearán por usar SCXtool?

SCXtool edita el texto de localización (las palabras que muestra el juego) y, desde la pestaña Game Settings, tus opciones de juego y asignaciones de teclas. No toca la lógica del juego, no te da ninguna ventaja ni se comunica con los servidores de CIG. Nuestras modificaciones **deberían** estar bien.

CIG ha respaldado públicamente la localización comunitaria. Su publicación [Community Localization Update](https://robertsspaceindustries.com/spectrum/community/SC/forum/1/thread/star-citizen-community-localization-update) expone el apoyo oficial a las traducciones hechas por jugadores, lo cual entendemos que permite explícitamente el tipo de edición de localización que hace SCXtool.

Streamers muy conocidos llevan a cabo proyectos de localización similares a plena vista, y a ninguno le han pedido que pare.

Dicho esto: el uso que le des a SCXtool es bajo tu propia responsabilidad. Nuestros cambios deberían estar bien, pero de cualquier cosa que hagas tú mismo, tú y tus asociados sois responsables de los daños que puedan producirse. Si alguna vez tienes dudas sobre si un cambio es apropiado, mantente en lo cosmético y guarda una copia de seguridad.

## ¿Qué archivos modifica SCXtool?

Solo estos, y únicamente cuando tú lo pides:

- `StarCitizen\<canal>\data\Localization\<idioma>\global.ini` — el archivo de localización del juego para el canal (LIVE, PTU, etc.) y el idioma que hayas seleccionado, que se escribe cuando pulsas **Aplicar mejoras**. SCXtool primero hace una copia de seguridad del archivo existente, y luego escribe el resultado combinado.
- `g_language` en tu `user.cfg`, que se define a la vez para que el juego cargue la localización correcta.
- `attributes.xml` (opciones de juego) y `actionmaps.xml` (asignaciones de teclas y opciones de mando) en `StarCitizen\<canal>\user\client\0\Profiles\default\`, que solo se escriben cuando pulsas **Save to Game** en la pestaña **Game Settings** o restauras una de sus copias de seguridad. SCXtool hace antes una copia de cada archivo y no escribe nada mientras el juego está en marcha.

Nada más en tu instalación del juego se toca.

Todo lo que SCXtool genera para su propio uso (la caché de origen, los archivos de mejoras, las copias de seguridad, tu `user.ini`) vive en tu carpeta de datos de SCXtool, no en el juego.

## ¿Por qué Windows dice que esta aplicación no es reconocida?

Porque SCXtool todavía no tiene firma de código. Windows SmartScreen y Smart App Control marcan cualquier aplicación nueva de un editor del que no tienen un certificado de firma registrado, incluso si es completamente segura. Es un aviso de "no hemos visto esto antes", no de "esto es peligroso".

Para ejecutarla: en el aviso de SmartScreen, pulsa **Más información → Ejecutar de todas formas**. Si Smart App Control la bloquea por completo, puedes permitir la aplicación desde su aviso, o desactivar Smart App Control temporalmente, instalar, y volver a activarlo.

La firma de código está en nuestra hoja de ruta, lo que hará que este aviso desaparezca. Mientras tanto, descarga SCXtool solo desde nuestras versiones oficiales de GitHub, para asegurarte de tener la compilación auténtica.
