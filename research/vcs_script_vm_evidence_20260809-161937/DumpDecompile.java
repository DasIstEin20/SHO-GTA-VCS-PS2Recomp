// Write a review-only decompilation for one or more existing functions.
//@category GTA.ScriptVM
// Usage: -postScript DumpDecompile.java <output-directory> <address> [address...]

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.BufferedWriter;
import java.nio.charset.StandardCharsets;

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DumpDecompile extends GhidraScript {
    private static String safeName(String value) {
        return value.replaceAll("[^A-Za-z0-9._-]", "_");
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2) {
            printerr("Usage: DumpDecompile.java <output-directory> <address> [address...]");
            return;
        }
        File directory = new File(args[0]);
        directory.mkdirs();
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);
        for (int index = 1; index < args.length; index++) {
            Address address = toAddr(args[index]);
            Function function = currentProgram.getFunctionManager().getFunctionAt(address);
            if (function == null) function = currentProgram.getFunctionManager().getFunctionContaining(address);
            if (function == null) throw new IllegalArgumentException("No function at " + args[index]);
            DecompileResults result = decompiler.decompileFunction(function, 90, monitor);
            String text = result.decompileCompleted() ? result.getDecompiledFunction().getC() : "/* DECOMPILATION FAILED: " + result.getErrorMessage() + " */\n";
            File output = new File(directory, safeName(function.getEntryPoint().toString()) + "_" + safeName(function.getName()) + ".c");
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(output), StandardCharsets.UTF_8));
            writer.write("/* REVIEW-ONLY: "); writer.write(function.getName()); writer.write(" @ 0x"); writer.write(function.getEntryPoint().toString()); writer.write(" */\n\n");
            writer.write(text);
            writer.close();
            println("Wrote " + output.getAbsolutePath());
        }
        decompiler.dispose();
    }
}
