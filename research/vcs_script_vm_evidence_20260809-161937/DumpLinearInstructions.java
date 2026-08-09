// Force a fixed-width MIPS review window to disassemble linearly, bypassing
// function-boundary recovery. It writes only review artifacts.
//@category GTA.ScriptVM
// Usage: -postScript DumpLinearInstructions.java <output-directory> <address> <instruction-count> [address count]...

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.Listing;

public class DumpLinearInstructions extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 3 || ((args.length - 1) % 2) != 0) {
            throw new IllegalArgumentException(
                "Usage: DumpLinearInstructions.java <output-directory> <address> <instruction-count> [address count]...");
        }
        File directory = new File(args[0]);
        directory.mkdirs();
        Listing listing = currentProgram.getListing();
        for (int index = 1; index < args.length; index += 2) {
            Address start = toAddr(args[index]);
            int count = Integer.decode(args[index + 1]);
            File output = new File(directory, start.toString() + "_linear_" + count + ".txt");
            try (BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(
                    new FileOutputStream(output), StandardCharsets.UTF_8))) {
                writer.write("REVIEW-ONLY forced-linear instruction window @ 0x" + start.toString() + "\n");
                writer.write("Program: " + currentProgram.getName() + "\n\n");
                for (int offset = 0; offset < count * 4; offset += 4) {
                    Address address = start.add(offset);
                    if (listing.getInstructionAt(address) == null) disassemble(address);
                    Instruction instruction = listing.getInstructionAt(address);
                    writer.write("0x" + address.toString() + "  " +
                        (instruction == null ? "<not decoded>" : instruction.toString()) + "\n");
                }
            }
            println("Wrote " + output.getAbsolutePath());
        }
    }
}
