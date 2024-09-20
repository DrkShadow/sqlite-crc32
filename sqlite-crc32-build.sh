gcc -fPIC -O2 -shared sqlite-crc32.c -o sqlite_crc32.so

echo "Installing file.."
sudo install -m 755 sqlite_crc32.so /usr/lib64/sqlite_crc32.so
