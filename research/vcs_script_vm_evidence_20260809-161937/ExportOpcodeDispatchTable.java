// Export the data-driven VCS opcode dispatch table used by ProcessOneCommand.
//@category GTA.ScriptVM
// Usage: -postScript ExportOpcodeDispatchTable.java <output-json> <table-address> <opcode-count> [dispatcher-address]

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.Memory;

public class ExportOpcodeDispatchTable extends GhidraScript {
    private static String json(String value) {
        if (value == null) return "null";
        StringBuilder out = new StringBuilder(value.length() + 8);
        out.append('"');
        for (int i = 0; i < value.length(); i++) {
            char c = value.charAt(i);
            switch (c) {
            case '\\': out.append("\\\\"); break;
            case '"': out.append("\\\""); break;
            case '\n': out.append("\\n"); break;
            case '\r': out.append("\\r"); break;
            case '\t': out.append("\\t"); break;
            default:
                if (c < 0x20) out.append(String.format("\\u%04x", (int)c));
                else out.append(c);
            }
        }
        out.append('"');
        return out.toString();
    }

    private static String hex(long value) {
        return String.format("0x%08x", value & 0xffffffffL);
    }

    private static long unsignedInt(Memory memory, Address address) throws Exception {
        return ((long)memory.getInt(address)) & 0xffffffffL;
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 3 || args.length > 4) throw new IllegalArgumentException("Usage: ExportOpcodeDispatchTable.java <output-json> <table-address> <opcode-count> [dispatcher-address]");
        File output = new File(args[0]);
        File parent = output.getParentFile();
        if (parent != null) parent.mkdirs();
        Address table = toAddr(args[1]);
        int count = Integer.decode(args[2]);
        String dispatcher = args.length == 4 ? args[3] : null;
        Memory memory = currentProgram.getMemory();
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(output), StandardCharsets.UTF_8));
        writer.write("{\"schema\":2,\"dispatcher\":"); writer.write(json(dispatcher)); writer.write(",\"table\":"); writer.write(json("0x" + table.toString()));
        writer.write(",\"entry_size\":8,\"opcode_first\":0,\"opcode_last\":"); writer.write(String.valueOf(count - 1));
        writer.write(",\"opcode_count_scanned\":"); writer.write(String.valueOf(count));
        // Preserve every slot, including zero records.  A consumer can therefore
        // distinguish an absent opcode from an omitted export record.
        writer.write(",\"slots\":[");
        for (int opcode = 0; opcode < count; opcode++) {
            if (opcode > 0) writer.write(",");
            Address entry = table.add((long)opcode * 8L);
            long descriptor = unsignedInt(memory, entry);
            long target = unsignedInt(memory, entry.add(4));
            boolean present = descriptor != 0 || target != 0;
            Function function = target == 0 ? null : currentProgram.getFunctionManager().getFunctionAt(toAddr(hex(target)));
            writer.write("{\"opcode\":"); writer.write(String.valueOf(opcode));
            writer.write(",\"entry_address\":"); writer.write(json(hex(entry.getOffset())));
            writer.write(",\"present\":"); writer.write(present ? "true" : "false");
            writer.write(",\"descriptor\":"); writer.write(json(hex(descriptor)));
            writer.write(",\"local_offset\":"); writer.write(String.valueOf((short)(descriptor & 0xffff)));
            writer.write(",\"selector\":"); writer.write(String.valueOf((short)((descriptor >>> 16) & 0xffff)));
            writer.write(",\"target\":"); writer.write(json(hex(target)));
            writer.write(",\"target_function\":"); writer.write(json(function == null ? null : function.getName()));
            writer.write("}");
        }
        writer.write("]");
        writer.write(",\"entries\":[");
        boolean first = true;
        int emitted = 0;
        for (int opcode = 0; opcode < count; opcode++) {
            Address entry = table.add((long)opcode * 8L);
            long descriptor = unsignedInt(memory, entry);
            long target = unsignedInt(memory, entry.add(4));
            if (descriptor == 0 && target == 0) continue;
            Function function = target == 0 ? null : currentProgram.getFunctionManager().getFunctionAt(toAddr(hex(target)));
            if (!first) writer.write(",");
            writer.write("{\"opcode\":"); writer.write(String.valueOf(opcode));
            writer.write(",\"descriptor\":"); writer.write(json(hex(descriptor)));
            writer.write(",\"local_offset\":"); writer.write(String.valueOf((short)(descriptor & 0xffff)));
            writer.write(",\"selector\":"); writer.write(String.valueOf((short)((descriptor >>> 16) & 0xffff)));
            writer.write(",\"target\":"); writer.write(json(hex(target)));
            writer.write(",\"target_function\":"); writer.write(json(function == null ? null : function.getName()));
            writer.write("}");
            first = false;
            emitted++;
        }
        writer.write("],\"nonzero_entries\":"); writer.write(String.valueOf(emitted)); writer.write("}");
        writer.close();
        println("Exported " + emitted + " opcode table records to " + output.getAbsolutePath());
    }
}
