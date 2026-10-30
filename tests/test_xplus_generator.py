import importlib.util
import os
from pathlib import Path
import tempfile
import unittest
import xml.etree.ElementTree as ET
import json

REPO = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('generator', REPO / 'framework/user-scripts/create-xplus-app.py')
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


class XPlusGeneratorTests(unittest.TestCase):
    def test_overwrite_updates_generated_files_preserving_extras_and_project_ids(self):
        with tempfile.TemporaryDirectory() as temporary:
            app = generator.create_app('refresh', 'refresh', Path(temporary) / 'app', True)
            project = app / 'client/Client.vcxproj'
            before = ET.parse(project).findtext('.//{*}ProjectGuid')
            desktop_before = ET.parse(app / 'desktop/Desktop.vcxproj').findtext('.//{*}ProjectGuid')
            (app / 'client/src/custom.hpp').write_text('VOLT_APP_NAME stays untouched')
            (app / 'client/src/App.x.hpp').write_text('old app')
            (app / '.git').mkdir()
            (app / '.git/sentinel').write_text('keep')
            generator.create_app('refresh', 'refresh', app, False, overwrite=True)
            self.assertNotEqual((app / 'client/src/App.x.hpp').read_text(), 'old app')
            self.assertEqual((app / 'client/src/custom.hpp').read_text(), 'VOLT_APP_NAME stays untouched')
            self.assertEqual((app / '.git/sentinel').read_text(), 'keep')
            self.assertEqual(ET.parse(project).findtext('.//{*}ProjectGuid'), before)
            self.assertIn(before, (app / 'refresh.sln').read_text())
            self.assertEqual(ET.parse(app / 'desktop/Desktop.vcxproj').findtext('.//{*}ProjectGuid'), desktop_before)

    def test_self_contained_app_and_unique_project_ids(self):
        with tempfile.TemporaryDirectory() as temporary:
            first = generator.create_app('my-app', 'test_v1', Path(temporary) / 'app with spaces', True)
            second = generator.create_app('second', 'second', Path(temporary) / 'second', True)
            for relative in ('client/src/App.x.hpp', 'client/src/components/Button.x.hpp',
                             'client/public/index.html', 'client/public/session.js', 'client/public/http.js',
                             'client/src/services/HttpClientService.hpp',
                             'desktop/main.cpp', 'desktop/DesktopServer.hpp', 'desktop/Desktop.vcxproj',
                             'tools/restore-webview2.ps1', 'DESKTOP.md', 'HTTP.md', 'server/controllers/ExampleController.hpp', 'server/services/HttpServerService.hpp',
                             'server/seasocks_impl.cpp', 'server/network/SessionHandler.hpp',
                             'server/sessions/Session.hpp', 'server/sessions/SessionDI.hpp',
                             'server/sessions/SessionBase.hpp',
                             'server/sessions/SessionCloseReason.hpp',
                             'server/sessions/Session.cpp',
                             'server/sessions/ISessionRegistry.hpp',
                             'server/sessions/SessionRegistry.hpp',
                             'server/sessions/services/SessionWebsocketService.hpp',
                             'shared/DependencyInjection.hpp', 'client/src/AppDI.hpp',
                             'shared/ActionMessage.hpp',
                             'shared/Talkers.hpp',
                             'shared/MemoryChanges.hpp', 'shared/MemoryStore.hpp', 'shared/MemoryViews.hpp',
                             'shared/examples/Student.hpp', 'MEMORY-STORE.md',
                             'shared/DataServices.hpp', 'shared/DataConnection.hpp', 'shared/MemoryWire.hpp', 'shared/ConnectionHeartbeat.hpp', 'DATA-SERVICES.md',
                             'client/src/services/VoltRuntimeService.hpp',
                             'client/src/services/ClientWebsocketService.hpp',
                             'client/src/ApplicationServices.hpp', 'server/AppDI.hpp',
                             'server/ApplicationServices.hpp',
                             'dependencies/volt/include/Volt.hpp', 'dependencies/volt/src/volt.js',
                             'dependencies/seasocks/LICENSE', 'dependencies/seasocks/generated/Embedded.cpp',
                             'tools/preprocesor.py', 'README.md', '.gitignore'):
                self.assertTrue((first / relative).is_file(), relative)
            self.assertFalse((first / 'output').exists())
            self.assertEqual(json.loads((first / 'app.json').read_text())['guid'], 'test_v1')
            self.assertIn('class MyApp', (first / 'client/src/App.x.hpp').read_text(encoding='utf-8'))
            ns = {'m': 'http://schemas.microsoft.com/developer/msbuild/2003'}
            ids = []
            for app in (first, second):
                client = ET.parse(app / 'client/Client.vcxproj')
                server = ET.parse(app / 'server/Server.vcxproj')
                client_id = client.find('.//m:ProjectGuid', ns).text
                server_id = server.find('.//m:ProjectGuid', ns).text
                desktop = ET.parse(app / 'desktop/Desktop.vcxproj')
                desktop_id = desktop.find('.//m:ProjectGuid', ns).text
                self.assertEqual(desktop.find('.//m:ProjectReference/m:Project', ns).text, server_id)
                self.assertEqual(server.find('.//m:ProjectReference/m:Project', ns).text, client_id)
                solution = next(app.glob('*.sln')).read_text()
                self.assertIn(client_id, solution)
                self.assertIn(server_id, solution)
                self.assertNotIn('VOLT_', solution)
                self.assertIn(desktop_id, solution)
                self.assertNotIn("VOLT_", (app / "desktop/main.cpp").read_text())
                ids.extend((client_id, server_id, desktop_id))
            self.assertEqual(len(set(ids)), 6)

    def test_existing_destination_and_invalid_names_are_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary)
            sentinel = path / 'keep.txt'
            sentinel.write_text('keep')
            with self.assertRaises(ValueError):
                generator.create_app('valid', 'valid', path, True)
            self.assertEqual(sentinel.read_text(), 'keep')
            for name in ('../escape', '1bad', 'name with spaces'):
                with self.assertRaises(ValueError):
                    generator.create_app(name, 'guid', path / 'unused', True)
            with self.assertRaises(ValueError):
                generator.create_app('valid', 'bad"guid', path / 'unused', True)
            self.assertFalse((path / 'unused').exists())

    def test_emsdk_path_is_validated_and_saved(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary)
            emsdk = path / 'sdk with spaces' / 'emsdk'
            emsdk.mkdir(parents=True)
            (emsdk / 'emsdk_env.bat').write_text('@rem stub')
            app = generator.create_app('sdk-app', 'sdk_app', path / 'app', True, emsdk_path=str(emsdk))
            saved = (app / 'emsdk-path.txt').read_text(encoding='utf-8').splitlines()
            self.assertEqual(saved, [str(emsdk)])
            self.assertIn('emsdk-path.txt', (app / '.gitignore').read_text().splitlines())
            # Relative paths resolve from the current directory and are saved absolute.
            previous = os.getcwd()
            os.chdir(emsdk.parent)
            try:
                relative = generator.create_app('rel-app', 'rel_app', path / 'rel', True, emsdk_path='emsdk')
            finally:
                os.chdir(previous)
            self.assertEqual((relative / 'emsdk-path.txt').read_text(encoding='utf-8').strip(), str(emsdk))
            # Refreshing without the option keeps the saved path; passing it replaces it.
            generator.create_app('sdk-app', 'sdk_app', app, True, overwrite=True)
            self.assertEqual((app / 'emsdk-path.txt').read_text(encoding='utf-8').strip(), str(emsdk))
            other = path / 'other-emsdk'
            other.mkdir()
            (other / 'emsdk_env.bat').write_text('@rem stub')
            generator.create_app('sdk-app', 'sdk_app', app, True, overwrite=True, emsdk_path=str(other))
            self.assertEqual((app / 'emsdk-path.txt').read_text(encoding='utf-8').strip(), str(other))

    def test_emsdk_path_omitted_or_invalid(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary)
            app = generator.create_app('plain', 'plain', path / 'plain', True)
            self.assertFalse((app / 'emsdk-path.txt').exists())
            not_sdk = path / 'not-sdk'
            not_sdk.mkdir()
            for value in (str(not_sdk), str(path / 'missing')):
                with self.assertRaisesRegex(ValueError, 'emsdk_env.bat'):
                    generator.create_app('bad', 'bad', path / 'bad', True, emsdk_path=value)
            self.assertFalse((path / 'bad').exists())


if __name__ == '__main__':
    unittest.main()
