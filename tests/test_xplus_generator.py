import importlib.util
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
    def test_self_contained_app_and_unique_project_ids(self):
        with tempfile.TemporaryDirectory() as temporary:
            first = generator.create_app('my-app', 'test_v1', Path(temporary) / 'app with spaces', True)
            second = generator.create_app('second', 'second', Path(temporary) / 'second', True)
            for relative in ('client/src/App.x.hpp', 'client/src/components/Button.x.hpp',
                             'client/public/index.html', 'server/seasocks_impl.cpp',
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
                self.assertEqual(server.find('.//m:ProjectReference/m:Project', ns).text, client_id)
                solution = next(app.glob('*.sln')).read_text()
                self.assertIn(client_id, solution)
                self.assertIn(server_id, solution)
                self.assertNotIn('VOLT_', solution)
                ids.extend((client_id, server_id))
            self.assertEqual(len(set(ids)), 4)

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


if __name__ == '__main__':
    unittest.main()
