// Export a strict, relocation-tolerant MIPS function fingerprint.
//@category GTA.ScriptVM
// Usage from analyzeHeadless:
//   -postScript ExportNormalizedMips.java <absolute-output-json>
//
// This intentionally does not try to identify source-level names.  It removes
// only target addresses / relocation-shaped constants while retaining opcode,
// register role, operand kind, small constants and delay-slot placement.  An
// equal normalized hash is therefore useful evidence, but still needs a
// separate semantic or call-graph anchor before a rename is permitted.

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.listing.Program;
import ghidra.program.model.lang.Register;
import ghidra.program.model.scalar.Scalar;

public class ExportNormalizedMips extends GhidraScript {
    private static final int SAMPLE_TOKENS = 12;

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

    private static String sha256(String value) throws Exception {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        byte[] bytes = value.getBytes(StandardCharsets.UTF_8);
        digest.update(bytes, 0, bytes.length);
        StringBuilder output = new StringBuilder();
        for (byte b : digest.digest()) output.append(String.format("%02x", b));
        return output.toString();
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

    private static String registerRole(Register register) {
        String name = register.getName().toLowerCase();
        if (name.equals("zero") || name.equals("r0")) return "zero";
        if (name.equals("sp")) return "sp";
        if (name.equals("gp")) return "gp";
        if (name.equals("ra")) return "ra";
        if (name.equals("at")) return "at";
        if (name.matches("a[0-3]")) return "arg";
        if (name.matches("v[0-1]")) return "ret";
        if (name.matches("s[0-7]")) return "saved";
        if (name.matches("t[0-9]")) return "temp";
        if (name.equals("k0") || name.equals("k1")) return "kernel";
        if (name.equals("hi") || name.equals("lo")) return name;
        return "gpr";
    }

    private static boolean controlMnemonic(String mnemonic) {
        return mnemonic.startsWith("b") || mnemonic.startsWith("j") || mnemonic.equals("eret");
    }

    private static String scalarRole(Scalar scalar, boolean control) {
        long value = scalar.getSignedValue();
        long absolute = value == Long.MIN_VALUE ? Long.MAX_VALUE : Math.abs(value);
        if (control) return "target";
        // Stack offsets, alignment, bytecode tags and small switches are part
        // of the semantic shape.  Large values are normally absolute data,
        // LUI/ORI relocation pieces or GP-relative material and are neutral.
        if (absolute <= 4095) return "imm:" + value;
        return "addr";
    }

    private static String fallback(String operand, boolean control) {
        String normalized = operand.toLowerCase();
        normalized = normalized.replaceAll("0x[0-9a-f]+", control ? "target" : "addr");
        normalized = normalized.replaceAll("[0-9a-f]{6,}h", control ? "target" : "addr");
        normalized = normalized.replaceAll("(?<![a-z0-9_])[0-9]{5,}(?![a-z0-9_])", control ? "target" : "addr");
        return normalized.replaceAll("\\s+", "");
    }

    private static String operandToken(Instruction instruction, int index, boolean control) {
        Object[] objects = instruction.getOpObjects(index);
        List<String> pieces = new ArrayList<String>();
        if (objects != null) for (Object object : objects) {
            if (object instanceof Register) pieces.add("reg:" + registerRole((Register)object));
            else if (object instanceof Scalar) pieces.add(scalarRole((Scalar)object, control));
            else if (object instanceof Address) pieces.add(control ? "target" : "addr");
            else if (object != null) pieces.add(fallback(object.toString(), control));
        }
        if (pieces.isEmpty()) pieces.add(fallback(instruction.getDefaultOperandRepresentation(index), control));
        return String.join("+", pieces);
    }

    private static void writeArray(BufferedWriter writer, List<String> values) throws Exception {
        writer.write("[");
        for (int index = 0; index < values.size(); index++) {
            if (index > 0) writer.write(",");
            writer.write(json(values.get(index)));
        }
        writer.write("]");
    }

    private static void writeCounter(BufferedWriter writer, Map<String, Integer> values) throws Exception {
        List<String> keys = new ArrayList<String>(values.keySet());
        Collections.sort(keys);
        writer.write("{");
        for (int index = 0; index < keys.size(); index++) {
            if (index > 0) writer.write(",");
            String key = keys.get(index);
            writer.write(json(key)); writer.write(":"); writer.write(String.valueOf(values.get(key)));
        }
        writer.write("}");
    }

    private void writeFunction(BufferedWriter writer, Function function, Listing listing) throws Exception {
        List<String> tokens = new ArrayList<String>();
        List<String> opcodes = new ArrayList<String>();
        List<String> controls = new ArrayList<String>();
        Map<String, Integer> histogram = new HashMap<String, Integer>();
        long branches = 0;
        long calls = 0;
        long loads = 0;
        long stores = 0;
        InstructionIterator iterator = listing.getInstructions(function.getBody(), true);
        while (iterator.hasNext()) {
            Instruction instruction = iterator.next();
            String mnemonic = instruction.getMnemonicString().toLowerCase();
            boolean control = controlMnemonic(mnemonic);
            StringBuilder token = new StringBuilder();
            token.append(instruction.isInDelaySlot() ? "delay:" : "op:");
            token.append(mnemonic);
            for (int index = 0; index < instruction.getNumOperands(); index++) {
                token.append("|");
                token.append(operandToken(instruction, index, control));
            }
            String value = token.toString();
            tokens.add(value);
            opcodes.add(mnemonic);
            if (control) controls.add(value);
            Integer prior = histogram.get(mnemonic);
            histogram.put(mnemonic, prior == null ? 1 : prior + 1);
            if (mnemonic.startsWith("b") || mnemonic.startsWith("j")) branches++;
            if (mnemonic.equals("jal") || mnemonic.equals("jalr")) calls++;
            if (mnemonic.startsWith("l")) loads++;
            if (mnemonic.startsWith("s")) stores++;
        }
        List<String> sample = new ArrayList<String>();
        for (int index = 0; index < Math.min(SAMPLE_TOKENS, tokens.size()); index++) sample.add(tokens.get(index));
        if (tokens.size() > SAMPLE_TOKENS * 2) sample.add("...");
        int tail = Math.max(SAMPLE_TOKENS, tokens.size() - SAMPLE_TOKENS);
        for (int index = tail; index < tokens.size(); index++) sample.add(tokens.get(index));

        writer.write("{\"address\":"); writer.write(json(hex(function.getEntryPoint())));
        writer.write(",\"name\":"); writer.write(json(function.getName()));
        writer.write(",\"size\":"); writer.write(String.valueOf(function.getBody().getNumAddresses()));
        writer.write(",\"instructions\":"); writer.write(String.valueOf(tokens.size()));
        writer.write(",\"branch_instructions\":"); writer.write(String.valueOf(branches));
        writer.write(",\"call_instructions\":"); writer.write(String.valueOf(calls));
        writer.write(",\"load_instructions\":"); writer.write(String.valueOf(loads));
        writer.write(",\"store_instructions\":"); writer.write(String.valueOf(stores));
        writer.write(",\"normalized_mips_sha256\":"); writer.write(json(sha256(String.join("\n", tokens))));
        writer.write(",\"opcode_sha256\":"); writer.write(json(sha256(String.join("\n", opcodes))));
        writer.write(",\"control_sha256\":"); writer.write(json(sha256(String.join("\n", controls))));
        writer.write(",\"opcode_histogram\":"); writeCounter(writer, histogram);
        writer.write(",\"sample_tokens\":"); writeArray(writer, sample);
        writer.write("}");
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 1) {
            printerr("Usage: ExportNormalizedMips.java <absolute-output-json>");
            return;
        }
        Program program = currentProgram;
        if (!program.getLanguageID().toString().toLowerCase().contains("mips")) {
            throw new IllegalArgumentException("ExportNormalizedMips requires a MIPS program, got " + program.getLanguageID());
        }
        File output = new File(args[0]);
        File parent = output.getParentFile();
        if (parent != null) parent.mkdirs();
        File executable = new File(program.getExecutablePath());
        String inputHash = executable.exists() ? sha256(executable) : null;
        Listing listing = program.getListing();
        FunctionIterator functions = program.getFunctionManager().getFunctions(true);
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(output), StandardCharsets.UTF_8));
        writer.write("{\"schema\":1,\"normalization\":\"mips-v1-relocation-neutral\",\"program\":");
        writer.write(json(program.getName()));
        writer.write(",\"sha256\":"); writer.write(json(inputHash));
        writer.write(",\"language\":"); writer.write(json(program.getLanguageID().toString()));
        writer.write(",\"functions\":[");
        boolean first = true;
        int count = 0;
        while (functions.hasNext()) {
            monitor.checkCancelled();
            Function function = functions.next();
            if (function.isExternal() || function.getBody().isEmpty()) continue;
            if (!first) writer.write(",");
            writeFunction(writer, function, listing);
            first = false;
            count++;
            if ((count % 250) == 0) println("Normalized " + count + " MIPS functions");
        }
        writer.write("]}");
        writer.close();
        println("Exported " + count + " normalized MIPS signatures to " + output.getAbsolutePath());
    }
}
