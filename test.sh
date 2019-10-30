echo "mounting filesystem on mount point mount"
./mount.myfs mount -l log.txt
echo "listing files"
cd mount
ls
echo "adding new file"
touch file0
echo "listing files"
ls
echo "adding another new file"
touch file1
echo "listing files"
ls
echo "removing first file"
rm file0
echo "listing"
ls
echo "listing file content"
cat file1
echo "adding file content"
echo "hello world" >> file1
echo "listing file content"
cat file1
echo "truncating file content two 2 bytes"
truncate -s 2 file1
echo "listing file content"
cat file1
echo "closing filesystem"
cd ..
fusermount --unmount mount

