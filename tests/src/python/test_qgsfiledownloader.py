# -*- coding: utf-8 -*-
"""
Test the QgsFileDownloader class

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from __future__ import print_function
from future import standard_library
import os
import tempfile
from functools import partial
from qgis.PyQt.QtCore import QEventLoop, QUrl, QTimer
from qgis.gui import (QgsFileDownloader,)
from qgis.testing import start_app, unittest

standard_library.install_aliases()

__author__ = 'Alessandro Pasotti'
__date__ = '08/11/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


start_app()


class TestQgsFileDownloader(unittest.TestCase):

    """
    This class tests the QgsFileDownloader class
    """

    def _make_download(self, url, destination, cancel=False):
        self.completed_was_called = False
        self.error_was_called = False
        self.canceled_was_called = False
        self.progress_was_called = False
        self.exited_was_called = False

        loop = QEventLoop()

        downloader = QgsFileDownloader(QUrl(url), destination, False)
        downloader.downloadCompleted.connect(partial(self._set_slot, 'completed'))
        downloader.downloadExited.connect(partial(self._set_slot, 'exited'))
        downloader.downloadCanceled.connect(partial(self._set_slot, 'canceled'))
        downloader.downloadError.connect(partial(self._set_slot, 'error'))
        downloader.downloadProgress.connect(partial(self._set_slot, 'progress'))

        downloader.downloadExited.connect(loop.quit)

        if cancel:
            downloader.downloadProgress.connect(downloader.onDownloadCanceled)

        loop.exec_()

    def test_validDownload(self):
        """Tests a valid download"""
        destination = tempfile.mktemp()
        self._make_download('http://www.qgis.org', destination)
        self.assertTrue(self.exited_was_called)
        self.assertTrue(self.completed_was_called)
        self.assertTrue(self.progress_was_called)
        self.assertFalse(self.canceled_was_called)
        self.assertFalse(self.error_was_called)
        self.assertTrue(os.path.isfile(destination))
        self.assertGreater(os.path.getsize(destination), 0)

    def test_inValidDownload(self):
        """Tests an invalid download"""
        destination = tempfile.mktemp()
        self._make_download('http://www.doesnotexistofthatimsure.qgis', destination)
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called)
        self.assertTrue(self.progress_was_called)
        self.assertFalse(self.canceled_was_called)
        self.assertTrue(self.error_was_called)
        self.assertEqual(self.error_args[1], [u'Network error 3: Host www.doesnotexistofthatimsure.qgis not found'])
        self.assertFalse(os.path.isfile(destination))

    def test_dowloadCanceled(self):
        """Tests user canceled download"""
        destination = tempfile.mktemp()
        self._make_download('https://github.com/qgis/QGIS/archive/master.zip', destination, True)
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called)
        self.assertTrue(self.canceled_was_called)
        self.assertFalse(self.error_was_called)
        self.assertFalse(os.path.isfile(destination))

    def test_InvalidUrl(self):
        destination = tempfile.mktemp()
        self._make_download('xyz://www', destination)
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called)
        self.assertFalse(self.canceled_was_called)
        self.assertTrue(self.error_was_called)
        self.assertFalse(os.path.isfile(destination))
        self.assertEqual(self.error_args[1], [u"Network error 301: Protocol \"xyz\" is unknown"])

    def test_InvalidFile(self):
        self._make_download('https://github.com/qgis/QGIS/archive/master.zip', "")
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called)
        self.assertFalse(self.canceled_was_called)
        self.assertTrue(self.error_was_called)
        self.assertEqual(self.error_args[1], [u"Cannot open output file: "])

    def test_BlankUrl(self):
        destination = tempfile.mktemp()
        self._make_download('', destination)
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called)
        self.assertFalse(self.canceled_was_called)
        self.assertTrue(self.error_was_called)
        self.assertFalse(os.path.isfile(destination))
        self.assertEqual(self.error_args[1], [u"Network error 301: Protocol \"\" is unknown"])

    def ssl_compare(self, name, url, error):
        destination = tempfile.mktemp()
        self._make_download(url, destination)
        msg = "Failed in %s: %s" % (name, url)
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called, msg)
        self.assertFalse(self.canceled_was_called, msg)
        self.assertTrue(self.error_was_called, msg)
        self.assertFalse(os.path.isfile(destination), msg)
        result = sorted(self.error_args[1])
        result = ';'.join(result)
        self.assertTrue(result.startswith(error), msg + "expected:\n%s\nactual:\n%s\n" % (result, error))

    def test_sslExpired(self):
        self.ssl_compare("expired", "https://expired.badssl.com/", "Network error 6: SSL handshake failed;SSL Errors: ;The certificate has expired")
        self.ssl_compare("self-signed", "https://self-signed.badssl.com/", "Network error 6: SSL handshake failed;SSL Errors: ;The certificate is self-signed, and untrusted")
        self.ssl_compare("untrusted-root", "https://untrusted-root.badssl.com/", "Network error 6: SSL handshake failed;No certificates could be verified;")

    def _set_slot(self, *args, **kwargs):
        #print('_set_slot(%s) called' % args[0])
        setattr(self, args[0] + '_was_called', True)
        setattr(self, args[0] + '_args', args)


if __name__ == '__main__':
    unittest.main()