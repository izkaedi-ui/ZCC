import pytest
import os
import sys
import struct
from unittest.mock import patch, mock_open, MagicMock

# Add current dir to path to import uf2conv
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import uf2conv

def test_is_uf2():
    # Valid UF2 magic
    valid_buf = struct.pack("<II", uf2conv.UF2_MAGIC_START0, uf2conv.UF2_MAGIC_START1) + b"\x00" * 504
    assert uf2conv.is_uf2(valid_buf) is True
    
    # Invalid UF2 magic
    invalid_buf = b"\x00" * 512
    assert uf2conv.is_uf2(invalid_buf) is False

def test_is_hex():
    assert uf2conv.is_hex(b":10010000214601360121470136007EFE09D2190140\r\n") is True
    assert uf2conv.is_hex(b"NOT HEX AT ALL") is False
    assert uf2conv.is_hex(b"\xff\xff\xff\xff") is False

@patch("uf2conv.load_families", return_value={"TEST_FAMILY": 0x12345678})
def test_convert_from_uf2(mock_load):
    uf2conv.appstartaddr = 0x2000
    uf2conv.familyid = 0x0
    # Create a valid block
    data = b"HELLO WORLD"
    flags = 0x2000 # has family ID
    curraddr = 0x2000
    family = 0x12345678
    
    hd = struct.pack(b"<IIIIIIII",
        uf2conv.UF2_MAGIC_START0, uf2conv.UF2_MAGIC_START1,
        flags, curraddr, len(data), 0, 1, family)
    
    padded_data = data + b"\x00" * (476 - len(data))
    block = hd + padded_data + struct.pack(b"<I", uf2conv.UF2_MAGIC_END)
    
    out = uf2conv.convert_from_uf2(block)
    assert out == data

    # Test bad magic
    bad_block = b"\x00" * 512
    assert uf2conv.convert_from_uf2(bad_block) == b""

    # Test flags NO-flash
    hd_no_flash = struct.pack(b"<IIIIIIII",
        uf2conv.UF2_MAGIC_START0, uf2conv.UF2_MAGIC_START1,
        1, curraddr, len(data), 0, 1, family)
    block_no_flash = hd_no_flash + padded_data + struct.pack(b"<I", uf2conv.UF2_MAGIC_END)
    assert uf2conv.convert_from_uf2(block_no_flash) == b""

def test_convert_to_carray():
    res = uf2conv.convert_to_carray(b"\x01\x02\x03")
    assert b"const unsigned long bindata_len = 3;" in res
    assert b"0x01, 0x02, 0x03, " in res

def test_convert_to_uf2():
    uf2conv.appstartaddr = 0x2000
    uf2conv.familyid = 0x0
    data = b"A" * 256
    out = uf2conv.convert_to_uf2(data)
    assert len(out) == 512
    assert uf2conv.is_uf2(out) is True

def test_Block_encode():
    b = uf2conv.Block(0x2000, 0xAA)
    enc = b.encode(0, 1)
    assert len(enc) == 512
    assert uf2conv.is_uf2(enc)

def test_convert_from_hex_to_uf2():
    hex_data = ":020000040000FA\n:04000000AABBCCDD00\n:00000001FF"
    out = uf2conv.convert_from_hex_to_uf2(hex_data)
    assert len(out) == 512
    assert uf2conv.is_uf2(out) is True

def test_to_str():
    assert uf2conv.to_str(b"hello") == "hello"

@patch("sys.platform", "win32")
@patch("subprocess.check_output")
@patch("os.path.isdir", return_value=True)
@patch("os.path.isfile", return_value=True)
def test_get_drives_win32(mock_isfile, mock_isdir, mock_check_output):
    mock_check_output.return_value = b"D:\r\nE:\r\n"
    drives = uf2conv.get_drives()
    assert drives == ["D:", "E:"]

@patch("builtins.open", new_callable=mock_open, read_data="Board-ID: NRF52-TEST\r\n")
def test_board_id(mock_file):
    assert uf2conv.board_id("/mnt/test") == "NRF52-TEST"

@patch("uf2conv.get_drives", return_value=["D:"])
@patch("uf2conv.board_id", return_value="BOARD-TEST")
@patch("builtins.print")
def test_list_drives(mock_print, mock_board_id, mock_get_drives):
    uf2conv.list_drives()
    mock_print.assert_called_with("D:", "BOARD-TEST")

@patch("builtins.open", new_callable=mock_open)
@patch("builtins.print")
def test_write_file(mock_print, mock_file):
    uf2conv.write_file("test.bin", b"DATA")
    mock_file.assert_called_with("test.bin", "wb")
    mock_file().write.assert_called_with(b"DATA")

@patch("builtins.open", new_callable=mock_open, read_data='[{"short_name": "FAM1", "id": "0x1111"}, {"short_name": "FAM2", "id": "0x2222"}]')
def test_load_families(mock_file):
    families = uf2conv.load_families()
    assert families["FAM1"] == 0x1111
    assert families["FAM2"] == 0x2222

@patch("sys.argv", ["uf2conv.py", "input.bin", "-b", "0x2000", "-c"])
@patch("builtins.open", new_callable=mock_open, read_data=b"0"*256)
@patch("uf2conv.write_file")
@patch("uf2conv.load_families", return_value={})
def test_main_convert(mock_load, mock_write, mock_file):
    uf2conv.main()
    mock_write.assert_called_once()

@patch("sys.argv", ["uf2conv.py", "-l"])
@patch("uf2conv.list_drives")
@patch("uf2conv.load_families", return_value={})
def test_main_list(mock_load, mock_list):
    uf2conv.main()
    mock_list.assert_called_once()
