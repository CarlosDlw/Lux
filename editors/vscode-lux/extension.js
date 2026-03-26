const { LanguageClient, TransportKind } = require("vscode-languageclient/node");
const vscode = require("vscode");

let client;

async function startClient(context) {
  const serverOptions = {
    command: "lux",
    args: ["lsp"],
    transport: TransportKind.stdio,
  };

  const clientOptions = {
    documentSelector: [{ scheme: "file", language: "lux" }],
    synchronize: {
      fileEvents: vscode.workspace.createFileSystemWatcher("**/*.lx"),
    },
  };

  client = new LanguageClient("lux", "Lux Language Server", serverOptions, clientOptions);
  await client.start();
}

function activate(context) {
  startClient(context);

  const restartCmd = vscode.commands.registerCommand("lux.restartServer", async () => {
    if (client) {
      await client.stop();
      client = undefined;
    }
    await startClient(context);
    vscode.window.showInformationMessage("Lux Language Server restarted.");
  });

  context.subscriptions.push(restartCmd);
  context.subscriptions.push({
    dispose: () => {
      if (client) {
        client.stop();
      }
    },
  });
}

function deactivate() {
  if (client) {
    return client.stop();
  }
}

module.exports = { activate, deactivate };
