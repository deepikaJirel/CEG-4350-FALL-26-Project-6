#include "project6.h"

#if PROJECT == 6
int kernel()
{
    startkernel(startshell);
    return 0;
}
#endif

// Delete the file that is currently opened
// All FAT entries for this file must be assigned a value of x0000
// Set the directory entry's first byte to x00
// Copy the last directory entry into this one if there are other files in this directory
// Returns 0 if file was successfully deleted
// Returns non-zero if error occurred
int deleteFile()
{
    // i. Check to see if currentFile is opened, if not return with error code (i.e. return -1)
    if(!currentFile.isOpened)
    {
        print("Error: No file was opened to be deleted.\n");
        return -1;
    }
    // ii. Overwrite all FAT clusters in the FAT table for this file with x0000
    uint16 cluster = currentFile.directoryEntry->startingCluster;
    while(cluster != 0xFFFF)
    {
    uint16 nextCluster = fat0->clusters[cluster];

        
        fat0->clusters[cluster] = 0x0000;
        fat1->clusters[cluster] = 0x0000;

    
        cluster = nextCluster;
    }
    // iii. Remove the directory entry that has the currentFile.directoryEntry->filename inside of it
    // at currentDirectory.startingAddress by overwriting the first element with NULL (x00) or by
    // overwriting entire directory entry with NULL
    // 1. Recommended: overwrite this directory entry with the last directory entry in the
    // directory
    
    directory_entry_t *lastEntry =
    (directory_entry_t *)currentDirectory.startingAddress;
        // While we are not at the end of the directory

    while(lastEntry->filename[0] != 0)
    {
        lastEntry++;
    }
    lastEntry--;

    // Overwrite the current file's directory entry with the last entry
    *currentFile.directoryEntry = *lastEntry;

    // Overwrite the first element with 0
    lastEntry->filename[0] = 0;

    // iv. Call writeFATs() to write the changes made to the FATs back to the disk
    if (writeFATs())
    {
        print("Error: Could not write FATs!\n");
        return -2;
    }
    // 1. If error occurs, return non zero
    // v. Call writeCurrentDirectory() to write the changes made to the current directory back to
    // the disk
        if (writeCurrentDirectory())
    {
        print("Error: Could not write current directory!\n");
        return -3;
    }
    // 1. If error occurs, return non zero
    // vi. Set currentFile.isOpened to 0 (false)
    currentFile.isOpened = 0;

    return 0;
}

// Delete the subdirectory inside of the directory that is currently opened
// Check to see if the subdirectory exists and is empty
// All FAT entries for this subdirectory must be assigned a value of x0000
// Set the directory entry's first byte to x00
// Copy the last directory entry into this one if there are other files in this directory
// Returns 0 if subdirectory was successfully deleted
// Returns non-zero if error occurred
int deleteSubdirectory(char *directoryName)
{
    // i. Check to see if currentDirectory is opened, if not return with error code (i.e. return -1)
if (!currentDirectory.isOpened)
{
    print("Error: Current directory is not opened!\n");
    return -1;
}
    // ii. Loop through the directory entries starting at currentDirectory.startingAddress and find
    // the entry with filename of directoryName
    // 1. If directoryName is not found, return non zero
    directory_entry_t *foundEntry=
    (directory_entry_t *)currentDirectory.startingAddress;
uint8 directoryWasNotFound = 1;

while (foundEntry->filename[0] != 0)
{
    if (stringcompare(
            (char *)foundEntry->filename,
            directoryName,
            8)
            &&
            ((foundEntry->attributes & 0x10) == 0x10))
        {
            directoryWasNotFound = 0;
            break;
        }

        foundEntry++;
    }

    if (directoryWasNotFound)
    {
        print("Error: Tried to delete a subdirectory that does not exist!\n");
        return -2;
    }
    // iii. Create a 512 byte buffer and load the subdirectory into it (we must check that the
    // subdirectory is empty before continuing)
    // 1. Note: you only need to load the first cluster of the subdirectory since the first two
    // entries should be “.” and “..” if the third directory is not empty, return non-zero
    // DO NOT DELETE THE SUBDIRECTORY IF IT CONTAINS FILES OR SUBDIRECTORIES
uint8 buffer[512] = {0};

    if (floppy_read(
            0,
            foundEntry->startingCluster + CLUSTER_TO_SECTOR,
            (void *)buffer,
            512))
    {
        print("Error: could not read sector!\n");
        return -3;
    }

    directory_entry_t *firstEntryInSubdir =
        (directory_entry_t *)buffer;

    firstEntryInSubdir += 2;

    if (firstEntryInSubdir->filename[0] != 0)
    {
        print("Error: subdirectory is not empty!\n");
        return -4;
    }

    // iv. Overwrite all FAT clusters in the FAT table for this subdirectory with x0000
uint16 cluster = foundEntry->startingCluster;

    while (cluster != 0xFFFF)
    {
        uint16 nextCluster = fat0->clusters[cluster];

        fat0->clusters[cluster] = 0x0000;
        fat1->clusters[cluster] = 0x0000;

        cluster = nextCluster;
    }
    // v. Remove the directory entry that has the directoryName inside of it at
    // currentDirectory.startingAddress by overwriting the first element with NULL (x00) or by
    // overwriting entire directory entry with NULL
    // 1. Recommended: overwrite this directory entry with the last directory entry in the
    // directory
        directory_entry_t *lastEntry =
        (directory_entry_t *)currentDirectory.startingAddress;

    while ((lastEntry + 1)->filename[0] != 0)
    {
        lastEntry++;
    }

    *foundEntry = *lastEntry;

    lastEntry->filename[0] = 0;
    // vi. Call writeFATs() to write the changes made to the FATs back to the disk
    // 1. If error occurs, return non zero
        if (writeFATs())
    {
        print("Error: Could not write the FATs to the disk after deleting subdirectory!\n");
        return -5;
    }
    // vii. Call writeCurrentDirectory() to write the changes made to the current directory back to
    // the disk
    // 1. If error occurs, return non zero
        if (writeCurrentDirectory())
    {
        print("Error: Could not write the current directory to the disk after deleting subdirectory!\n");
        return -6;
    }

    return 0;
}