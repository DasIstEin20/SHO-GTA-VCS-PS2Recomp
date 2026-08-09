// Dump review-only instruction windows at opcode-handler entry labels.
//@category GTA.ScriptVM
// Usage: -postScript DumpInstructionWindows.java <output-directory> <address> [address...]

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;

public class DumpInstructionWindows extends GhidraScript {
    private static String safe(String value) { return value.replaceAll("[^A-Za-z0-9._-]", "_"); }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2) throw new IllegalArgumentException("Usage: DumpInstructionWindows.java <output-directory> <address> [address...]");
        File directory = new File(args[0]);
        directory.mkdirs();
        Listing listing = currentProgram.getListing();
        for (int index = 1; index < args.length; index++) {
            Address start = toAddr(args[index]);
            if (listing.getInstructionAt(start) == null) disassemble(start);
            File output = new File(directory, safe(start.toString()) + "_instruction_window.txt");
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(output), StandardCharsets.UTF_8));
            writer.write("REVIEW-ONLY instruction window @ 0x" + start.toString() + "\n");
            writer.write("Program: " + currentProgram.getName() + "\n\n");
            InstructionIterator iterator = listing.getInstructions(start, true);
            int emitted = 0;
            boolean stopAfterDelaySlot = false;
            while (iterator.hasNext() && emitted < 180) {
                monitor.checkCancelled();
                Instruction instruction = iterator.next();
                writer.write("0x" + instruction.getAddress().toString() + "  " + instruction.toString());
                writer.write("  ; flow=" + instruction.getFlowType().toString() + "\n");
                emitted++;
                if (stopAfterDelaySlot) break;
                // A MIPS CALL_TERMINATOR returns to the next instruction and
                // must not truncate the review window.  For a real return/jump
                // include its delay slot, then stop before the next handler.
                if (instruction.getFlowType().isTerminal() && !instruction.getFlowType().isCall()) {
                    if (instruction.getDelaySlotDepth() == 0) break;
                    stopAfterDelaySlot = true;
                }
            }
            writer.close();
            println("Wrote " + output.getAbsolutePath());
        }
    }
}
