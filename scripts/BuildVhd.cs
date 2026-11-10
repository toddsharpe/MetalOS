#:package LTRData.DiscUtils.Vhdx@1.0.90
#:package LTRData.DiscUtils.Fat@1.0.90
#:property PublishAot=false

using DiscUtils.Fat;
using DiscUtils.Partitions;
using DiscUtils.Streams;

//Builds the boot disk without mounting it, so without admin: a dynamic VHDX holding a GPT with an EFI system partition for what the loader loads (under EFI\BOOT) and a MetalOS partition for everything else (at its root)
const long EspSize = 512L * 1024 * 1024; //DiscUtils formats FAT32 only from 512 MB; a dynamic disk only stores what's written
const long DataSize = 1024L * 1024 * 1024;

const long SectorSize = 512;
const long FirstSector = 1024 * 1024 / SectorSize;
const long DataFirstSector = FirstSector + EspSize / SectorSize;
const long DiskSize = EspSize + DataSize + 2 * 1024 * 1024;
const string BootDirectory = @"EFI\BOOT";
const string OsSeparator = "--os";

int separator = Array.IndexOf(args, OsSeparator);
if (args.Length < 2 || separator < 2)
{
	Console.Error.WriteLine($"usage: dotnet run --file BuildVhd.cs -- <output.vhdx> <boot file>... {OsSeparator} <os file>...");
	return 1;
}

string output = Path.GetFullPath(args[0]);
string[] bootFiles = args[1..separator];
string[] osFiles = args[(separator + 1)..];
foreach (string file in bootFiles.Concat(osFiles))
{
	if (!File.Exists(file))
	{
		Console.Error.WriteLine($"BuildVhd: missing {file}");
		return 1;
	}
}

//Built next to the output and swapped in at the end, so a failed run never leaves a half-written disk
string temp = output + ".tmp";
using (FileStream stream = new FileStream(temp, FileMode.Create, FileAccess.ReadWrite))
using (DiscUtils.Vhdx.Disk disk = DiscUtils.Vhdx.Disk.InitializeDynamic(stream, Ownership.None, DiskSize))
{
	GuidPartitionTable gpt = GuidPartitionTable.Initialize(disk);
	int esp = gpt.Create(FirstSector, DataFirstSector - 1, GuidPartitionTypes.EfiSystem, 0, "EFI system partition");
	int data = gpt.Create(DataFirstSector, DataFirstSector + DataSize / SectorSize - 1, GuidPartitionTypes.WindowsBasicData, 0, "MetalOS");

	if (!Populate(disk, esp, "SYSTEM", BootDirectory, bootFiles) || !Populate(disk, data, "METALOS", "", osFiles))
		return 1;
}

File.Move(temp, output, true);
Console.WriteLine($"BuildVhd: wrote {output}");
return 0;

//Formats the partition FAT32 and copies the files into the directory
static bool Populate(DiscUtils.Vhdx.Disk disk, int partition, string label, string directory, string[] files)
{
	using FatFileSystem fat = FatFileSystem.FormatPartition(disk, partition, label);
	if (fat.FatVariant != FatType.Fat32)
	{
		Console.Error.WriteLine($"BuildVhd: {label} formatted {fat.FatVariant}, expected Fat32");
		return false;
	}

	if (directory.Length != 0)
		fat.CreateDirectory(directory);
	foreach (string file in files)
	{
		string target = Path.Combine(directory, Path.GetFileName(file));
		using FileStream source = File.OpenRead(file);
		using SparseStream destination = fat.OpenFile(target, FileMode.Create, FileAccess.Write);
		source.CopyTo(destination);
	}

	Console.WriteLine($"BuildVhd: {label}: {files.Length} files on {fat.FatVariant} ({fat.ClusterSize}-byte clusters)");
	return true;
}
