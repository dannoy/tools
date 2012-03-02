#!/bin/sh

prog=./iodisk

#file=$(echo testfile.{0,1,2,3,4,5})
#file="testfile.0"
file="test.data"
#file="GHOSTXP.GHO"
#file="test.ghost"
#file="/mnt/usb/test.data"
#file="/opt/ftp/pub/os/GHOSTXP.GHO"
#SUDO=sudo

read_block_size="1k 4k 512k 1m"
write_block_size="1k 4k 512k 1m"
write_file_size="100m 1g"

#sync_method=$(seq 0 5)
#sync_method="0:sync() 1:fsync() 2:fdatasync() 3:O_DIRECT 4:O_SYNC 5:O_DIRECT|O_SYNC"
#sync_method="0:sync() 1:fsync() 2:fdatasync() 3:O_DIRECT 5:O_DIRECT|O_SYNC"
sync_method="0:sync() 3:O_DIRECT"
#sync_desp[0]="sync()"
#sync_desp[1]="fsync()"
#sync_desp[2]="fdatasync()"
#sync_desp[3]="O_DIRECT"
#sync_desp[4]="O_SYNC"
#sync_desp[5]="O_DIRECT | O_SYNC"

test_rounds=5


echo -n Preparing data
dd if=/dev/zero of=$file bs=1024M count=1 > /dev/null 2>&1
#dd if=/dev/zero of=$file bs=100M count=1 > /dev/null 2>&1
echo ".... ok"
echo ======== Test Read ==========
for blk_size in $read_block_size
do
    echo -e "\033[47;34m $blk_size: \033[0m"
    $prog --file $file --read --block_size $blk_size --rounds $test_rounds
done


echo ======== Test Read Randomly ==========
for blk_size in $read_block_size
do
    echo -e "\033[47;34m $blk_size: \033[0m"
    $prog --file $file --read --block_size $blk_size --rounds $test_rounds --random
done

#for blk_size in $read_block_size
#do
    #echo -e "\033[47;34m $blk_size mmap: \033[0m"
    #$prog --file $file --read --block_size $blk_size --rounds $test_rounds --mmap --random
#done

echo ======== 'Test Read & Write'  ==========
for blk_size in $read_block_size
do
    echo -e "\033[47;34m $blk_size: \033[0m"
    $prog --file $file --read_write --block_size $blk_size --rounds $test_rounds --random --sync 0
done

#for blk_size in $read_block_size
#do
    #echo -e "\033[47;34m $blk_size mmap: \033[0m"
    #$prog --file $file --read_write --block_size $blk_size --rounds $test_rounds --mmap --random
#done

echo ======== Test Write ==========
for blk_size in $write_block_size
do
    for sync in $sync_method
    do
        #echo ${sync_desp[$sync]}:
        for f_size in $write_file_size
        do
            echo -e "\033[47;34m $blk_size ${sync##*:} $f_size:\033[0m"
            $prog --file $file --write --block_size $blk_size --rounds $test_rounds --sync ${sync%%:*} --file_size $f_size
        done
    done
done

echo ======== Test Write Randomly==========
for blk_size in $write_block_size
do
    for sync in $sync_method
    do
        #echo ${sync_desp[$sync]}:
        for f_size in $write_file_size
        do
            echo -e "\033[47;34m $blk_size ${sync##*:} $f_size:\033[0m"
            $prog --file $file --write --block_size $blk_size --rounds $test_rounds --sync ${sync%%:*} --file_size $f_size --random
        done
    done
done

