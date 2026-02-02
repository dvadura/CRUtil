import pytest

from hello import Hello

class TestHello():
   EXPECTED = '<PYTHON> Hello World!'

   def test_hello(self):
      assert Hello().speak(True) == TestHello.EXPECTED
