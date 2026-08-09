// Export MIPS instruction-level signals for Script VM dispatcher research.
//@category GTA.ScriptVM
// Usage: -postScript ExportScriptVmSemanticSignals.java <absolute-output-json>
//
// This does not score functions. It preserves the concrete signals needed to
// identify a dispatcher or a refactored fetch/mask/page-selection chain:
// halfword opcode reads, +2 updates, high-bit handling, 0x7fff stripping,
// range thresholds and direct callgraph edges.

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.listing.Program;
import ghidra.program.model.scalar.Scalar;

public class ExportScriptVmSemanticSignals extends GhidraScript {
    private static final int MAX_LINES = 80;

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

    private static String hex(Address address) {
        return address == null ? null : "0x" + address.toString();
    }

    private static String sha256(File input) throws Exception {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        java.io.FileInputStream stream = new java.io.FileInputStream(input);
        byte[] buffer = new byte[1024 * 128];
        int read;
        while ((read = stream.read(buffer)) != -1) digest.update(buffer, 0, read);
        stream.close();
        StringBuilder output = new StringBuilder();
        for (byte b : digest.digest()) output.append(String.format("%02x", b));
        return output.toString();
    }

    private static List<Long> scalars(Instruction instruction) {
        List<Long> output = new ArrayList<Long>();
        for (int index = 0; index < instruction.getNumOperands(); index++) {
            Object[] objects = instruction.getOpObjects(index);
            if (objects == null) continue;
            for (Object object : objects) {
                if (object instanceof Scalar) {
                    Scalar scalar = (Scalar)object;
                    long value = scalar.getUnsignedValue();
                    if (!output.contains(value)) output.add(value);
                }
            }
        }
        return output;
    }

    private static boolean contains(List<Long> values, long wanted) {
        for (Long value : values) if (value.longValue() == wanted) return true;
        return false;
    }

    private static boolean rangeThreshold(long value) {
        return value >= 64 && value <= 4096 && (value % 10 == 0 || value % 100 == 0);
    }

    private static boolean relevant(String mnemonic, List<Long> values) {
        if (mnemonic.equals("lh") || mnemonic.equals("lhu")) return true;
        if ((mnemonic.equals("addi") || mnemonic.equals("addiu")) && contains(values, 2)) return true;
        if ((mnemonic.equals("andi") || mnemonic.equals("and")) && (contains(values, 0x7fff) || contains(values, 0x8000))) return true;
        if ((mnemonic.equals("srl") || mnemonic.equals("sra") || mnemonic.equals("sll")) && contains(values, 15)) return true;
        if (mnemonic.equals("bltz") || mnemonic.equals("bgez") || mnemonic.equals("bltzal") || mnemonic.equals("bgezal")) return true;
        for (Long value : values) if (rangeThreshold(value.longValue())) return true;
        return false;
    }

    private static void array(BufferedWriter writer, List<String> values) throws Exception {
        writer.write("[");
        for (int index = 0; index < values.size(); index++) {
            if (index > 0) writer.write(",");
            writer.write(json(values.get(index)));
        }
        writer.write("]");
    }

    private static void numbers(BufferedWriter writer, Set<Long> values) throws Exception {
        List<Long> sorted = new ArrayList<Long>(values);
        Collections.sort(sorted);
        writer.write("[");
        for (int index = 0; index < sorted.size(); index++) {
            if (index > 0) writer.write(",");
            writer.write(String.valueOf(sorted.get(index)));
        }
        writer.write("]");
    }

    private static final class Signal {
        Function function;
        int instructions;
        int halfwordLoads;
        int incrementTwo;
        int highBit;
        int stripMask;
        int rangeComparisons;
        Set<Long> thresholds = new HashSet<Long>();
        List<String> lines = new ArrayList<String>();
        List<String> callees = new ArrayList<String>();
        List<String> callers = new ArrayList<String>();
    }

    private Signal inspect(Function function, Listing listing) throws Exception {
        Signal value = new Signal();
        value.function = function;
        InstructionIterator iterator = listing.getInstructions(function.getBody(), true);
        while (iterator.hasNext()) {
            Instruction instruction = iterator.next();
            value.instructions++;
            String mnemonic = instruction.getMnemonicString().toLowerCase();
            List<Long> constants = scalars(instruction);
            if (mnemonic.equals("lh") || mnemonic.equals("lhu")) value.halfwordLoads++;
            if ((mnemonic.equals("addi") || mnemonic.equals("addiu")) && contains(constants, 2)) value.incrementTwo++;
            if ((mnemonic.equals("andi") && contains(constants, 0x8000)) ||
                    ((mnemonic.equals("srl") || mnemonic.equals("sra")) && contains(constants, 15)) ||
                    mnemonic.equals("bltz") || mnemonic.equals("bgez")) value.highBit++;
            if (mnemonic.equals("andi") && contains(constants, 0x7fff)) value.stripMask++;
            for (Long constant : constants) {
                if (rangeThreshold(constant.longValue())) {
                    value.thresholds.add(constant);
                    if (mnemonic.startsWith("slt") || mnemonic.startsWith("b") || mnemonic.equals("addiu")) value.rangeComparisons++;
                }
            }
            if (relevant(mnemonic, constants) && value.lines.size() < MAX_LINES) {
                value.lines.add(hex(instruction.getAddress()) + " " + instruction.toString());
            }
        }
        for (Function callee : function.getCalledFunctions(monitor)) value.callees.add(hex(callee.getEntryPoint()));
        for (Function caller : function.getCallingFunctions(monitor)) value.callers.add(hex(caller.getEntryPoint()));
        Collections.sort(value.callees);
        Collections.sort(value.callers);
        return value;
    }

    private static boolean emit(Signal value) {
        // The broad fetch arm and broad page-selection arm are both retained;
        // an analysis consumer joins them through calls/callers if VCS split
        // them across helpers.
        boolean fetch = value.halfwordLoads > 0 && (value.incrementTwo > 0 || value.highBit > 0 || value.stripMask > 0);
        boolean select = value.thresholds.size() >= 2 || (value.rangeComparisons >= 2 && value.callees.size() >= 3);
        return fetch || select;
    }

    private static void write(BufferedWriter writer, Signal value) throws Exception {
        writer.write("{\"address\":"); writer.write(json(hex(value.function.getEntryPoint())));
        writer.write(",\"name\":"); writer.write(json(value.function.getName()));
        writer.write(",\"instructions\":"); writer.write(String.valueOf(value.instructions));
        writer.write(",\"halfword_loads\":"); writer.write(String.valueOf(value.halfwordLoads));
        writer.write(",\"increment_two\":"); writer.write(String.valueOf(value.incrementTwo));
        writer.write(",\"high_bit_signals\":"); writer.write(String.valueOf(value.highBit));
        writer.write(",\"strip_0x7fff\":"); writer.write(String.valueOf(value.stripMask));
        writer.write(",\"range_comparisons\":"); writer.write(String.valueOf(value.rangeComparisons));
        writer.write(",\"range_thresholds\":"); numbers(writer, value.thresholds);
        writer.write(",\"callees\":"); array(writer, value.callees);
        writer.write(",\"callers\":"); array(writer, value.callers);
        writer.write(",\"semantic_instruction_lines\":"); array(writer, value.lines);
        writer.write("}");
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 1) throw new IllegalArgumentException("Usage: ExportScriptVmSemanticSignals.java <absolute-output-json>");
        // PSP uses a MIPS language ID while the installed Emotion Engine
        // extension identifies PS2 R5900 differently.  Both expose the MIPS
        // mnemonics inspected below, so do not reject the PS2 program here.
        File output = new File(args[0]);
        File parent = output.getParentFile();
        if (parent != null) parent.mkdirs();
        Program program = currentProgram;
        Listing listing = program.getListing();
        List<Signal> signals = new ArrayList<Signal>();
        FunctionIterator iterator = program.getFunctionManager().getFunctions(true);
        while (iterator.hasNext()) {
            monitor.checkCancelled();
            Function function = iterator.next();
            if (function.isExternal() || function.getBody().isEmpty()) continue;
            Signal signal = inspect(function, listing);
            if (emit(signal)) signals.add(signal);
        }
        Collections.sort(signals, (a, b) -> a.function.getEntryPoint().compareTo(b.function.getEntryPoint()));
        File executable = new File(program.getExecutablePath());
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(output), StandardCharsets.UTF_8));
        writer.write("{\"schema\":1,\"program\":"); writer.write(json(program.getName()));
        writer.write(",\"language\":"); writer.write(json(program.getLanguageID().toString()));
        writer.write(",\"sha256\":"); writer.write(json(executable.exists() ? sha256(executable) : null));
        writer.write(",\"method\":\"instruction-level dispatcher semantic signals; unranked\",\"functions\":[");
        for (int index = 0; index < signals.size(); index++) {
            if (index > 0) writer.write(",");
            write(writer, signals.get(index));
        }
        writer.write("]}");
        writer.close();
        println("Exported " + signals.size() + " semantic-signal functions to " + output.getAbsolutePath());
    }
}
