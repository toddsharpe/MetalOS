#:package LTRData.DiscUtils.Vhdx@1.0.90
#:package LTRData.DiscUtils.Fat@1.0.90
#:property PublishAot=false

using DiscUtils.Fat;
using DiscUtils.Partitions;
using DiscUtils.Streams;

//Builds the boot disk without mounting it, so without admin: a dynamic VHDX holding a GPT with one EFI system partition, every file under EFI\BOOT
const long PartitionSize = 512L * 1024 * 1024; //DiscUtils formats FAT32 (what diskpart used) only from 512 MB; a dynamic disk only stores what's written

const long SectorSize = 512;
const long FirstSector = 1024 * 1024 / SectorSize;
const long DiskSize = PartitionSize + 2 * 1024 * 1024;
const string BootDirectory = @"EFI\BOOT";

if (args.Length < 2)
{
	Console.Error.WriteLine("usage: dotnet run --file MakeVhd.cs -- <output.vhdx> <file>...");
	return 1;
}

string output = Path.GetFullPath(args[0]);
string[] files = args[1..];
foreach (string file in files)
{
	if (!File.Exists(file))
	{
		Console.Error.WriteLine($"MakeVhd: missing {file}");
		return 1;
	}
}

//Built next to the output and swapped in at the end, so a failed run never leaves a half-written disk
string temp = output + ".tmp";
using (FileStream stream = new FileStream(temp, FileMode.Create, FileAccess.ReadWrite))
using (DiscUtils.Vhdx.Disk disk = DiscUtils.Vhdx.Disk.InitializeDynamic(stream, Ownership.None, DiskSize))
{
	GuidPartitionTable gpt = GuidPartitionTable.Initialize(disk);
	int partition = gpt.Create(FirstSector, FirstSector + PartitionSize / SectorSize - 1, GuidPartitionTypes.EfiSystem, 0, "EFI system partition");

	using FatFileSystem fat = FatFileSystem.FormatPartition(disk, partition, "SYSTEM");
	if (fat.FatVariant != FatType.Fat32)
	{
		Console.Error.WriteLine($"MakeVhd: formatted {fat.FatVariant}, expected Fat32");
		return 1;
	}

	fat.CreateDirectory(BootDirectory);
	foreach (string file in files)
	{
		string target = Path.Combine(BootDirectory, Path.GetFileName(file));
		using FileStream source = File.OpenRead(file);
		using SparseStream destination = fat.OpenFile(target, FileMode.Create, FileAccess.Write);
		source.CopyTo(destination);
	}

	Console.WriteLine($"MakeVhd: {files.Length} files on {fat.FatVariant} ({fat.ClusterSize}-byte clusters)");
}

File.Move(temp, output, true);
Console.WriteLine($"MakeVhd: wrote {output}");
return 0;
