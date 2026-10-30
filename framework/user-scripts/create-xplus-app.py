#!/usr/bin/env python3
"""Shared x+ scaffolding for the PowerShell and Bash entry points."""
import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import uuid
import xml.etree.ElementTree as ET

# Read by tools/build-client.cmd (and Client.vcxproj for IntelliSense).
EMSDK_PATH_FILE = 'emsdk-path.txt'


def resolve_emsdk_path(value):
    """Return the absolute emsdk folder for --emsdk-path, or None when omitted."""
    if not value:
        return None
    path = Path(os.path.abspath(os.path.expanduser(value)))
    if not (path / 'emsdk_env.bat').is_file():
        raise ValueError(f'--emsdk-path must name the emsdk folder that contains emsdk_env.bat: {path}')
    return path


def create_app(name, guid, destination, no_git, overwrite=False, emsdk_path=None):
    if not re.fullmatch(r'[A-Za-z][A-Za-z0-9_-]*', name):
        raise ValueError('App name must start with a letter and contain only letters, digits, _ or -')
    if not re.fullmatch(r'[A-Za-z0-9_-]+', guid):
        raise ValueError('GUID must contain only letters, digits, _ or -')
    emsdk = resolve_emsdk_path(emsdk_path)
    repo = Path(__file__).resolve().parents[2]
    destination = Path(destination).resolve() if destination else repo.parent / name
    existed = destination.exists()
    if existed and not overwrite:
        raise ValueError(f'Output directory already exists: {destination}')
    template = repo / 'app-template-x-plus'
    if destination == repo or destination in repo.parents or destination.is_relative_to(template) or destination.is_relative_to(repo / 'framework'):
        raise ValueError('Output directory overlaps generator sources')
    def project_guid(relative):
        project = destination / relative
        if overwrite and project.is_file():
            value = ET.parse(project).findtext('.//{*}ProjectGuid')
            if value:
                return str(uuid.UUID(value.strip('{}'))).upper()
        return str(uuid.uuid4()).upper()
    client_guid = project_guid('client/Client.vcxproj')
    server_guid = project_guid('server/Server.vcxproj')
    desktop_guid = project_guid('desktop/Desktop.vcxproj')
    shutil.copytree(template, destination, dirs_exist_ok=overwrite)
    shutil.copytree(repo / 'framework/include', destination / 'dependencies/volt/include', dirs_exist_ok=overwrite)
    shutil.copytree(repo / 'framework/src', destination / 'dependencies/volt/src', dirs_exist_ok=overwrite)
    shutil.copy2(repo / 'app-template-x/preprocesor.py', destination / 'tools/preprocesor.py')
    tokens = {
        'VOLT_APP_NAME_CAMEL': ''.join(word.capitalize() for word in re.split('[-_]', name)),
        'VOLT_APP_NAME_UNDERSCORE': name.replace('-', '_'),
        'VOLT_APP_NAME': name,
        'VOLT_CLIENT_PROJECT_GUID': client_guid,
        'VOLT_SERVER_PROJECT_GUID': server_guid,
        'VOLT_DESKTOP_PROJECT_GUID': desktop_guid,
    }
    # Only our template files are substituted, never dependency sources/binaries.
    files = [destination / file.relative_to(template) for file in (template / 'client').rglob('*')]
    files += [destination / file.relative_to(template) for file in (template / 'server').glob('*.vcxproj')]
    files += [destination / file.relative_to(template) for file in (template / 'desktop').rglob('*')]
    files += [destination / 'VOLT_APP_NAME.sln']
    for file in files:
        if not file.is_file() or file.suffix not in ('.cpp', '.hpp', '.h', '.c', '.vcxproj', '.sln', '.html'):
            continue
        content = file.read_text(encoding='utf-8')
        for token, replacement in tokens.items():
            content = content.replace(token, replacement)
        if file.name == 'index.html':
            content = content.replace('Volt App', name)
        file.write_text(content, encoding='utf-8')
    (destination / 'VOLT_APP_NAME.sln').replace(destination / f'{name}.sln')
    (destination / 'app.json').write_text(json.dumps({'name': name, 'guid': guid}, indent=2) + '\n', encoding='utf-8')
    if emsdk:
        # Machine-specific, so the template's .gitignore excludes it. On --overwrite
        # without --emsdk-path an existing file is kept.
        (destination / EMSDK_PATH_FILE).write_text(f'{emsdk}\n', encoding='utf-8')
    if not no_git and not existed:
        for args in (['init', '-q'], ['add', '.'], ['commit', '-q', '-m', 'Initial commit - Created with Volt X+']):
            subprocess.run(['git', *args], cwd=destination, check=True)
    print(f'Created Volt X+: {destination}')
    if emsdk:
        print(f'Emscripten SDK: {emsdk} (saved in {EMSDK_PATH_FILE})')
        if not str(emsdk).isascii():
            print('WARNING: tools/build-client.cmd reads emsdk-path.txt with the console code page, '
                  'so non-ASCII characters in this path may not survive. Prefer an ASCII-only SDK path.')
    print(f'Open {name}.sln in Visual Studio 2026. Build with Ctrl+Shift+B; run Server with F5.')
    print('Browse http://127.0.0.1:8000/ (WebSocket echo: /ws), or set Desktop as startup project for a native window.')
    return destination


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('name')
    parser.add_argument('--guid', default='')
    parser.add_argument('--output', default='')
    parser.add_argument('--no-git', action='store_true')
    parser.add_argument('--emsdk-path', default='',
                        help='emsdk folder (containing emsdk_env.bat) to save in the app for Visual Studio builds')
    parser.add_argument('--overwrite', action='store_true', help=argparse.SUPPRESS)
    args = parser.parse_args()
    try:
        create_app(args.name, args.guid or args.name, args.output, args.no_git, args.overwrite, args.emsdk_path)
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'ERROR: {error}\n')
