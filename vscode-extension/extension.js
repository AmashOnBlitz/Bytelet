const vscode = require('vscode')

const COLOR_SCOPES = [
  { id: 'keyword', scope: 'keyword', label: 'Keywords (let, if, else, for, while, return...)', default: '#C586C0' },
  { id: 'import', scope: 'keyword.control.import', label: 'import keyword', default: '#FFD700' },
  { id: 'var', scope: 'keyword.control.var', label: 'var keyword', default: '#90EE90' },
  { id: 'fn', scope: 'keyword.control.function', label: 'fn keyword', default: '#4EC9B0' },
  { id: 'type', scope: 'storage.type', label: 'Types (int, float, bool, string, void)', default: '#DCDCAA' },
  { id: 'function', scope: 'entity.name.function', label: 'Functions (print, len, ...)', default: '#569CD6' },
  { id: 'variable', scope: 'variable.parameter', label: 'Variables & identifiers', default: '#FF8080' },
  { id: 'number', scope: 'constant.numeric', label: 'Numbers', default: '#B5CEA8' },
  { id: 'string', scope: 'string.quoted.double', label: 'Strings', default: '#CE9178' },
  { id: 'operator', scope: 'keyword.operator', label: 'Operators (+, =, ==, ->, ...)', default: '#D4D4D4' },
  { id: 'comment', scope: 'comment', label: 'Comments', default: '#D896FF' }
]

function applyColors() {
  const config = vscode.workspace.getConfiguration('bytelite')
  const rules = []

  for (const c of COLOR_SCOPES) {
    const color = config.get(`color.${c.id}`) || c.default
    rules.push({ scope: c.scope, settings: { foreground: color } })
  }

  vscode.workspace.getConfiguration('editor').update('tokenColorCustomizations', { textMateRules: rules }, true)
}

async function pickColor(item) {
  const config = vscode.workspace.getConfiguration('bytelite')
  const current = config.get(`color.${item.id}`) || item.default

  const input = await vscode.window.showInputBox({
    prompt: `Enter hex color for ${item.label}`,
    value: current,
    placeHolder: 'e.g. #FF0000',
    validateInput: (text) => /^#([0-9A-Fa-f]{3}|[0-9A-Fa-f]{6})$/.test(text) ? null : 'Use a valid hex color like #FF0000'
  })

  if (input) {
    await config.update(`color.${item.id}`, input, true)
    applyColors()
    vscode.window.showInformationMessage(`Bytelite: ${item.label} set to ${input}`)
  }
}

function activate(context) {
  applyColors()

  context.subscriptions.push(
    vscode.workspace.onDidChangeConfiguration(e => {
      if (e.affectsConfiguration('bytelite.color.')) applyColors()
    })
  )

  context.subscriptions.push(
    vscode.commands.registerCommand('bytelite.configureColors', async () => {
      const picked = await vscode.window.showQuickPick(COLOR_SCOPES.map(c => ({
        label: c.label,
        description: c.default,
        id: c.id
      })), {
        title: 'Bytelite: Choose element to recolor',
        placeHolder: 'Pick a category, then enter a hex color'
      })

      if (picked) {
        const item = COLOR_SCOPES.find(c => c.id === picked.id)
        if (item) await pickColor(item)
      }
    })
  )

  context.subscriptions.push(
    vscode.commands.registerCommand('bytelite.resetColors', async () => {
      const config = vscode.workspace.getConfiguration('bytelite')
      for (const c of COLOR_SCOPES) {
        await config.update(`color.${c.id}`, c.default, true)
      }
      applyColors()
      vscode.window.showInformationMessage('Bytelite: Colors reset to defaults')
    })
  )
}

function deactivate() {}

module.exports = { activate, deactivate }
