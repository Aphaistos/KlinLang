import * as path from 'path';
import { workspace, ExtensionContext, window } from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: ExtensionContext) {
    // Récupération de la configuration pour un chemin personnalisé éventuel
    const config = workspace.getConfiguration('klin');
    let serverExecutable = config.get<string>('lsp.path');

    // Résolution du binaire par défaut si aucun chemin n'est fourni
    if (!serverExecutable) {
        const platform = process.platform;
        const binaryName = platform === 'win32' ? 'klinc-lsp.exe' : 'klinc-lsp';
        
        // Emplacement du binaire dans le dossier de l'extension (bin/klinc-lsp)
        serverExecutable = context.asAbsolutePath(path.join('bin', binaryName));
    }

    // Configuration des options d'exécution du serveur
    const serverOptions: ServerOptions = {
        run: { 
            command: serverExecutable, 
            transport: TransportKind.stdio 
        },
        debug: { 
            command: serverExecutable, 
            transport: TransportKind.stdio,
            options: { env: { ...process.env, KLIN_LOG_LEVEL: 'DEBUG' } }
        }
    };

    // Configuration des options du client VS Code
    const clientOptions: LanguageClientOptions = {
        // Ciblage des fichiers du langage Klin (.kln)
        documentSelector: [
            { scheme: 'file', language: 'klin' },
            { scheme: 'untitled', language: 'klin' }
        ],
        synchronize: {
            // Recharger si les fichiers de configuration de projet ou les manifests de modules changent
            fileEvents: [
                workspace.createFileSystemWatcher('**/klin.json'),
                workspace.createFileSystemWatcher('**/*.kln')
            ]
        },
        // Canal d'affichage dédié dans l'onglet Output de VS Code
        outputChannelName: 'Klin Language Server'
    };

    // Instanciation du client LSP
    client = new LanguageClient(
        'klinLSP',
        'Klin Language Server',
        serverOptions,
        clientOptions
    );

    // Démarrage du client et connexion au serveur
    client.start().catch((error) => {
        window.showErrorMessage(
            `Impossible de démarrer le serveur LSP Klin (${serverExecutable}): ${error.message}`
        );
    });
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}