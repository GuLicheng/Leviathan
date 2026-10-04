import json
import itertools

file = "C:\\Users\\Administrator\\.vscode\\extensions\\dracula-theme.theme-dracula-2.25.1\\theme\\dracula.json"

class ScopeSetting:

    def __init__(self, scope, settings):
        self.scope = scope
        self.settings =settings

    def __str__(self):
        return f"scope={self.scope}, settings={self.settings}"

def read_scope_settings(file):
    with open(file, "r") as f:
        data = json.load(f)

    scope_settings = []
    for item in data["tokenColors"]:
        scope = item.get("scope")
        settings = item.get("settings")
        scope_settings.append(ScopeSetting(scope, settings))

    return scope_settings

def extract_italic_scope_settings(scope_settings):
    italic_scope_settings = []
    for setting in scope_settings:
        if setting.settings and setting.settings.get("fontStyle") == "italic":
            italic_scope_settings.append(setting)
    return italic_scope_settings

def main():

    scope_settings = read_scope_settings(file)
    italic_scope_settings = extract_italic_scope_settings(scope_settings)
    print("\nItalic Scope Settings:")
    scopes = itertools.chain.from_iterable(map(lambda x : x.scope, italic_scope_settings))
    for scope in scopes:
        print(f'"{scope}",')

if __name__ == "__main__":
    main()
