# Aspectos legais

> Esta página é uma tradução fornecida apenas para fins informativos. **A versão em inglês prevalece**; em caso de divergência, o texto em inglês (e os arquivos `LICENSE` e `NOTICE` distribuídos ao lado do executável) tem precedência.

## Star Citizen / Cloud Imperium

O SCXtool é uma **ferramenta comunitária não oficial** para Star Citizen. Ele não é desenvolvido, endossado ou patrocinado pela Cloud Imperium Games (CIG) ou pela Roberts Space Industries (RSI), nem é afiliado a elas.

**Star Citizen®**, **Roberts Space Industries®** e **Cloud Imperium®** são marcas registradas da Cloud Imperium Rights LLC e da Cloud Imperium Rights Ltd. Todos os dados do jogo Star Citizen, incluindo o conteúdo do `Data.p4k`, são propriedade intelectual da Cloud Imperium Rights LLC.

O SCXtool não redistribui nenhum conteúdo da CIG ou da RSI. Ele lê arquivos da **sua própria instalação licenciada de Star Citizen** e grava seus textos personalizados de volta nessa instalação. Nada do jogo sai do seu computador através do SCXtool.

## Licenças

O SCXtool é derivado do Smart Citizen (Licença Apache 2.0, © Osiris DevWorks e colaboradores) e do unp4k (Licença MIT, © dolkensp e colaboradores). Os textos completos das licenças e as atribuições acompanham os arquivos `LICENSE` e `NOTICE` ao lado do executável.

Bibliotecas de terceiros:

- **Qt 6**: The Qt Company, usada sob a **GNU LGPL v3**, com vinculação dinâmica.
- **zstd**: Meta Platforms, licença **BSD**.
- **zlib**: Jean-loup Gailly e Mark Adler, licença **zlib**.
- **pugixml**: Arseny Kapoulkine, licença **MIT**.

O software é fornecido **"no estado em que se encontra"**, sem garantias ou condições de qualquer tipo.

## Privacidade

O SCXtool é um aplicativo de desktop local. Ele não envia telemetria, análises ou relatórios de falhas, e não tem contas.

Ele só se conecta à internet para verificar no GitHub se há uma versão mais recente do SCXtool (a cada inicialização e quando você clica em **Verificar por atualizações**) e baixá-la se você decidir atualizar, para baixar o `global.ini` comunitário de um idioma quando você muda para esse idioma, e para buscar um arquivo INI de uma URL quando você importa um. A verificação de atualizações não envia nada sobre você ou seu PC além da versão do app. Suas edições, configurações, backups e caches ficam no seu computador:

- **Configurações**: `%APPDATA%\SCXtool\settings.ini` (ou `data\settings.ini` ao lado do executável nas versões portáteis).
- **Suas edições, backups e cache de textos**: `Documents\SCXtool\<canal>\` por padrão (configurável na aba Config).
- **Cache do DataForge**: `%LOCALAPPDATA%\SCXtool\<canal>\cache\dataforge\`.
- **Logs e relatórios de falhas**: `Documents\SCXtool\logs\`.

## Use por sua conta e risco

O SCXtool altera o texto de localização, algo que a CIG apoia para traduções comunitárias, e as opções de jogo e os controles que você escolher na aba Game Settings. Como você o usa é responsabilidade sua.
