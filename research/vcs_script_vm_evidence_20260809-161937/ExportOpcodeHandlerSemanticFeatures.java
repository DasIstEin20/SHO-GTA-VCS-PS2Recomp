// Export review-oriented semantic features for Script-VM opcode handlers.
//@category GTA.ScriptVM
// Usage: -postScript ExportOpcodeHandlerSemanticFeatures.java <targets-tsv> <output-json>
//
// targets-tsv format: one `opcode<TAB>handler-address` row per handler.  The
// same address may occur for several opcodes and is exported once with all of
// its slots.  This is deliberately an evidence export: it records MIPS-level
// facts and does not assign a source command name or classify a difference.

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.scalar.Scalar;

public class ExportOpcodeHandlerSemanticFeatures extends GhidraScript {
    private static final int MAX_LINEAR_INSTRUCTIONS = 192;
    private static final int MAX_FUNCTION_INSTRUCTIONS = 8192;
    private static final Pattern MEMORY = Pattern.compile("(-?(?:0x[0-9a-fA-F]+|[0-9]+))\\(([^)]+)\\)");

    private static class Handler {
        String target;
        List<Integer> opcodes = new ArrayList<Integer>();
    }

    private static class Feature {
        String target;
        String source;
        String function;
        String functionEntry;
        boolean truncated;
        int instructions;
        List<String> parameterDecoders = new ArrayList<String>();
        List<String> threadAccesses = new ArrayList<String>();
        List<String> globalAccesses = new ArrayList<String>();
        List<String> globalPattern = new ArrayList<String>();
        List<Long> constants = new ArrayList<Long>();
        List<String> callees = new ArrayList<String>();
        Map<String, Integer> calleeKinds = new TreeMap<String, Integer>();
        Map<String, Integer> returnSemantics = new TreeMap<String, Integer>();
        int blocks;
        int conditionalBranches;
        int jumps;
        int calls;
        int returnSites;
        int loads;
        int stores;
    }

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
            default: if (c < 0x20) out.append(String.format("\\u%04x", (int)c)); else out.append(c);
            }
        }
        out.append('"');
        return out.toString();
    }

    private static String hex(Address address) {
        return address == null ? null : "0x" + address.toString();
    }

    private static String hex(long value) {
        return String.format("0x%08x", value & 0xffffffffL);
    }

    private static String signedHex(long value) {
        return value < 0 ? String.format("-0x%x", -value) : String.format("+0x%x", value);
    }

    private static String sha256(File input) throws Exception {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        FileInputStream stream = new FileInputStream(input);
        byte[] buffer = new byte[1024 * 128];
        int read;
        while ((read = stream.read(buffer)) != -1) digest.update(buffer, 0, read);
        stream.close();
        StringBuilder output = new StringBuilder();
        for (byte b : digest.digest()) output.append(String.format("%02x", b));
        return output.toString();
    }

    private static String register(Instruction instruction, int operand) {
        Object[] objects = instruction.getOpObjects(operand);
        if (objects != null) for (Object object : objects) if (object instanceof Register) return ((Register)object).getName().toLowerCase();
        return null;
    }

    private static Long scalar(Instruction instruction, int operand) {
        Object[] objects = instruction.getOpObjects(operand);
        if (objects != null) for (Object object : objects) if (object instanceof Scalar) return ((Scalar)object).getSignedValue();
        return null;
    }

    private static boolean isZero(String value) {
        return value != null && (value.equals("zero") || value.equals("r0"));
    }

    private static String mnemonic(Instruction instruction) {
        return instruction.getMnemonicString().replace("_", "").toLowerCase();
    }

    private static boolean isCall(String op) { return op.equals("jal") || op.equals("jalr") || op.equals("bal"); }
    private static boolean isReturn(Instruction instruction, String op) { return op.equals("jr") && "ra".equals(register(instruction, 0)); }
    private static boolean isConditionalBranch(String op) { return op.startsWith("b") && !op.equals("bal") && !op.equals("break"); }
    private static boolean isJump(Instruction instruction, String op) { return op.equals("j") || (op.equals("jr") && !"ra".equals(register(instruction, 0))); }

    private static String memoryKind(String op) {
        if (op.startsWith("lb") || op.startsWith("sb")) return op.startsWith("l") ? "r8" : "w8";
        if (op.startsWith("lh") || op.startsWith("sh")) return op.startsWith("l") ? "r16" : "w16";
        if (op.startsWith("ld") || op.startsWith("sd")) return op.startsWith("l") ? "r64" : "w64";
        if (op.startsWith("lw") || op.startsWith("sw")) return op.startsWith("l") ? "r32" : "w32";
        if (op.startsWith("lwc") || op.startsWith("swc")) return op.startsWith("l") ? "r32" : "w32";
        if (op.startsWith("ldc") || op.startsWith("sdc")) return op.startsWith("l") ? "r64" : "w64";
        return null;
    }

    private static Long parseNumber(String value) {
        try {
            boolean negative = value.startsWith("-");
            String digits = negative ? value.substring(1) : value;
            long parsed = digits.toLowerCase().startsWith("0x") ? Long.parseLong(digits.substring(2), 16) : Long.parseLong(digits);
            return negative ? -parsed : parsed;
        } catch (Exception ignored) { return null; }
    }

    private static void increment(Map<String, Integer> values, String key) {
        Integer prior = values.get(key);
        values.put(key, prior == null ? 1 : prior + 1);
    }

    private static void addConstant(List<Long> output, Instruction instruction, int operand, String op) {
        if (isCall(op) || isConditionalBranch(op) || isJump(instruction, op)) return;
        String representation = instruction.getDefaultOperandRepresentation(operand);
        if (representation.contains("(")) return; // Memory displacement is represented as an access field, not a semantic constant.
        Long value = scalar(instruction, operand);
        if (value != null && Math.abs(value) <= 0xffffL) output.add(value);
    }

    private static Long constantWrite(Instruction instruction, String op) {
        String destination = register(instruction, 0);
        if (!"v0".equals(destination)) return null;
        if (op.equals("move") || op.equals("or")) {
            String source = register(instruction, 1);
            String other = register(instruction, 2);
            if (isZero(source) || isZero(other)) return 0L;
        }
        if (op.equals("li")) return scalar(instruction, 1);
        if (op.equals("ori") || op.equals("addiu") || op.equals("daddiu")) {
            String source = register(instruction, 1);
            if (isZero(source)) return scalar(instruction, 2);
        }
        return null;
    }

    private List<Instruction> instructionsFor(Handler handler, Listing listing, Feature feature) {
        Address start = toAddr(handler.target);
        Function function = currentProgram.getFunctionManager().getFunctionAt(start);
        if (function == null) function = currentProgram.getFunctionManager().getFunctionContaining(start);
        List<Instruction> values = new ArrayList<Instruction>();
        if (function != null && function.getBody().contains(start)) {
            feature.source = "function_body";
            feature.function = function.getName();
            feature.functionEntry = hex(function.getEntryPoint());
            InstructionIterator iterator = listing.getInstructions(function.getBody(), true);
            while (iterator.hasNext()) {
                Instruction instruction = iterator.next();
                values.add(instruction);
                if (values.size() >= MAX_FUNCTION_INSTRUCTIONS) { feature.truncated = true; break; }
            }
            return values;
        }
        feature.source = "linear_until_return";
        if (listing.getInstructionAt(start) == null) disassemble(start);
        InstructionIterator iterator = listing.getInstructions(start, true);
        boolean stopAfterDelay = false;
        while (iterator.hasNext() && values.size() < MAX_LINEAR_INSTRUCTIONS) {
            Instruction instruction = iterator.next();
            values.add(instruction);
            if (stopAfterDelay) break;
            String op = mnemonic(instruction);
            if (isReturn(instruction, op)) {
                if (instruction.getDelaySlotDepth() == 0) break;
                stopAfterDelay = true;
            }
        }
        if (values.size() >= MAX_LINEAR_INSTRUCTIONS) feature.truncated = true;
        return values;
    }

    private Feature extract(Handler handler, Listing listing) throws Exception {
        Feature feature = new Feature();
        feature.target = handler.target;
        List<Instruction> instructions = instructionsFor(handler, listing, feature);
        feature.instructions = instructions.size();
        Map<String, Long> thread = new HashMap<String, Long>();
        thread.put("a0", 0L);
        Map<String, Long> constants = new HashMap<String, Long>();
        Map<String, Long> globals = new HashMap<String, Long>();
        Set<Long> blocks = new HashSet<Long>();
        if (!instructions.isEmpty()) blocks.add(instructions.get(0).getAddress().getOffset());
        Long v0 = null;
        for (int index = 0; index < instructions.size(); index++) {
            monitor.checkCancelled();
            Instruction instruction = instructions.get(index);
            String op = mnemonic(instruction);
            String destination = instruction.getNumOperands() > 0 ? register(instruction, 0) : null;
            for (int operand = 0; operand < instruction.getNumOperands(); operand++) addConstant(feature.constants, instruction, operand, op);
            String access = memoryKind(op);
            if (access != null) {
                if (access.startsWith("r")) feature.loads++; else feature.stores++;
                for (int operand = 0; operand < instruction.getNumOperands(); operand++) {
                    Matcher memory = MEMORY.matcher(instruction.getDefaultOperandRepresentation(operand));
                    if (!memory.find()) continue;
                    Long displacement = parseNumber(memory.group(1));
                    String base = memory.group(2).trim().toLowerCase();
                    if (displacement == null) continue;
                    Long threadBase = thread.get(base);
                    if (threadBase != null && threadBase == 0) feature.threadAccesses.add(access + "@" + signedHex(displacement));
                    else if (globals.containsKey(base)) {
                        long absolute = globals.get(base) + displacement;
                        feature.globalAccesses.add(access + "@" + hex(absolute));
                        feature.globalPattern.add(access);
                    } else if (base.equals("gp")) {
                        feature.globalAccesses.add(access + "@gp" + signedHex(displacement));
                        feature.globalPattern.add(access);
                    }
                }
            }
            if (isCall(op)) {
                feature.calls++;
                Long parameter = thread.get("a1");
                String kind = parameter != null && parameter == 0x10L ? "script_param_decoder" : "direct_call";
                Long decoderType = constants.get("a2");
                if (kind.equals("script_param_decoder") && decoderType != null) kind += ":type=" + decoderType;
                Address[] flows = instruction.getFlows();
                String target = flows != null && flows.length > 0 ? hex(flows[0]) : "indirect";
                feature.callees.add(kind + "@" + target);
                increment(feature.calleeKinds, kind);
                if (kind.startsWith("script_param_decoder")) feature.parameterDecoders.add(kind);
            }
            if (isConditionalBranch(op)) feature.conditionalBranches++;
            if (isJump(instruction, op)) feature.jumps++;
            if (isReturn(instruction, op)) {
                feature.returnSites++;
                Long returned = v0;
                if (index + 1 < instructions.size()) {
                    Long delayValue = constantWrite(instructions.get(index + 1), mnemonic(instructions.get(index + 1)));
                    if (delayValue != null) returned = delayValue;
                }
                increment(feature.returnSemantics, returned == null ? "unknown" : "constant:" + returned);
            }
            Address[] flows = instruction.getFlows();
            if ((isConditionalBranch(op) || isJump(instruction, op)) && flows != null) for (Address flow : flows) blocks.add(flow.getOffset());
            if (isConditionalBranch(op)) {
                Instruction next = instruction.getNext();
                if (next != null) blocks.add(next.getAddress().getOffset());
            }
            // Update lightweight register provenance after collecting the facts
            // for this instruction.  This tracks `move s0,a0` and
            // `addiu a1,s0,0x10`, the common Script-VM parameter decoder setup.
            if (destination != null) {
                thread.remove(destination);
                constants.remove(destination);
                globals.remove(destination);
                if (op.equals("move") || op.equals("or") || op.equals("addu") || op.equals("daddu")) {
                    String source = register(instruction, 1);
                    String other = register(instruction, 2);
                    if (thread.containsKey(source) && (other == null || isZero(other))) thread.put(destination, thread.get(source));
                    else if (thread.containsKey(other) && isZero(source)) thread.put(destination, thread.get(other));
                    if (constants.containsKey(source) && (other == null || isZero(other))) constants.put(destination, constants.get(source));
                    else if (constants.containsKey(other) && isZero(source)) constants.put(destination, constants.get(other));
                }
                if (op.equals("addiu") || op.equals("daddiu")) {
                    String source = register(instruction, 1);
                    Long immediate = scalar(instruction, 2);
                    if (immediate != null && thread.containsKey(source)) thread.put(destination, thread.get(source) + immediate);
                    if (immediate != null && globals.containsKey(source)) globals.put(destination, globals.get(source) + immediate);
                    if (immediate != null && isZero(source)) constants.put(destination, immediate);
                }
                if (op.equals("ori")) {
                    String source = register(instruction, 1);
                    Long immediate = scalar(instruction, 2);
                    if (immediate != null && globals.containsKey(source)) globals.put(destination, globals.get(source) | (immediate & 0xffffL));
                    if (immediate != null && isZero(source)) constants.put(destination, immediate);
                }
                if (op.equals("li")) { Long immediate = scalar(instruction, 1); if (immediate != null) constants.put(destination, immediate); }
                if (op.equals("lui")) { Long immediate = scalar(instruction, 1); if (immediate != null) globals.put(destination, (immediate & 0xffffL) << 16); }
                Long directV0 = constantWrite(instruction, op);
                if ("v0".equals(destination)) v0 = directV0;
            }
        }
        feature.blocks = blocks.size();
        Collections.sort(feature.parameterDecoders);
        Collections.sort(feature.threadAccesses);
        Collections.sort(feature.globalAccesses);
        Collections.sort(feature.globalPattern);
        Collections.sort(feature.constants);
        Collections.sort(feature.callees);
        return feature;
    }

    private static void strings(BufferedWriter writer, List<String> values) throws Exception {
        writer.write("["); for (int i = 0; i < values.size(); i++) { if (i > 0) writer.write(","); writer.write(json(values.get(i))); } writer.write("]");
    }
    private static void numbers(BufferedWriter writer, List<Long> values) throws Exception {
        writer.write("["); for (int i = 0; i < values.size(); i++) { if (i > 0) writer.write(","); writer.write(String.valueOf(values.get(i))); } writer.write("]");
    }
    private static void counter(BufferedWriter writer, Map<String, Integer> values) throws Exception {
        writer.write("{"); boolean first = true; for (Map.Entry<String, Integer> entry : values.entrySet()) { if (!first) writer.write(","); writer.write(json(entry.getKey())); writer.write(":"); writer.write(String.valueOf(entry.getValue())); first = false; } writer.write("}");
    }
    private static void opcodes(BufferedWriter writer, List<Integer> values) throws Exception {
        writer.write("["); for (int i = 0; i < values.size(); i++) { if (i > 0) writer.write(","); writer.write(String.valueOf(values.get(i))); } writer.write("]");
    }
    private static void writeFeature(BufferedWriter writer, Handler handler, Feature value) throws Exception {
        writer.write("{\"target\":"); writer.write(json(value.target)); writer.write(",\"opcodes\":"); opcodes(writer, handler.opcodes);
        writer.write(",\"source\":"); writer.write(json(value.source)); writer.write(",\"function\":"); writer.write(json(value.function)); writer.write(",\"function_entry\":"); writer.write(json(value.functionEntry));
        writer.write(",\"truncated\":"); writer.write(value.truncated ? "true" : "false"); writer.write(",\"instructions\":"); writer.write(String.valueOf(value.instructions));
        writer.write(",\"parameter_decoder_calls\":"); strings(writer, value.parameterDecoders); writer.write(",\"thread_accesses\":"); strings(writer, value.threadAccesses);
        writer.write(",\"global_accesses\":"); strings(writer, value.globalAccesses); writer.write(",\"global_access_pattern\":"); strings(writer, value.globalPattern);
        writer.write(",\"small_constants\":"); numbers(writer, value.constants); writer.write(",\"callees\":"); strings(writer, value.callees); writer.write(",\"callee_kinds\":"); counter(writer, value.calleeKinds);
        writer.write(",\"cfg\":{\"basic_blocks_approx\":"); writer.write(String.valueOf(value.blocks)); writer.write(",\"conditional_branches\":"); writer.write(String.valueOf(value.conditionalBranches)); writer.write(",\"jumps\":"); writer.write(String.valueOf(value.jumps)); writer.write(",\"call_sites\":"); writer.write(String.valueOf(value.calls)); writer.write(",\"return_sites\":"); writer.write(String.valueOf(value.returnSites)); writer.write(",\"loads\":"); writer.write(String.valueOf(value.loads)); writer.write(",\"stores\":"); writer.write(String.valueOf(value.stores)); writer.write("}");
        writer.write(",\"return_semantics\":"); counter(writer, value.returnSemantics); writer.write("}");
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 2) throw new IllegalArgumentException("Usage: ExportOpcodeHandlerSemanticFeatures.java <targets-tsv> <output-json>");
        File input = new File(args[0]);
        File output = new File(args[1]);
        Map<String, Handler> handlers = new TreeMap<String, Handler>();
        BufferedReader reader = new BufferedReader(new FileReader(input));
        String line;
        while ((line = reader.readLine()) != null) {
            line = line.trim(); if (line.isEmpty() || line.startsWith("#")) continue;
            String[] columns = line.split("\\t");
            if (columns.length != 2) throw new IllegalArgumentException("Invalid target TSV row: " + line);
            int opcode = columns[0].toLowerCase().startsWith("0x") ? Integer.parseInt(columns[0].substring(2), 16) : Integer.parseInt(columns[0]);
            String target = columns[1].toLowerCase();
            Handler handler = handlers.get(target);
            if (handler == null) { handler = new Handler(); handler.target = target; handlers.put(target, handler); }
            handler.opcodes.add(opcode);
        }
        reader.close();
        for (Handler handler : handlers.values()) Collections.sort(handler.opcodes);
        File parent = output.getParentFile(); if (parent != null) parent.mkdirs();
        Listing listing = currentProgram.getListing();
        File executable = new File(currentProgram.getExecutablePath());
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(output), StandardCharsets.UTF_8));
        writer.write("{\"schema\":1,\"purpose\":\"review-only normalized handler semantics; no automatic semantic classification\",\"program\":"); writer.write(json(currentProgram.getName()));
        writer.write(",\"language\":"); writer.write(json(currentProgram.getLanguageID().toString())); writer.write(",\"sha256\":"); writer.write(json(executable.exists() ? sha256(executable) : null));
        writer.write(",\"normalization\":\"mips-handler-v1: decoder calls, direct thread fields, global read/write shape, small constants, call kinds, approximate CFG and return constants\",\"handlers\":[");
        boolean first = true; int count = 0;
        for (Handler handler : handlers.values()) {
            monitor.checkCancelled();
            if (!first) writer.write(","); writeFeature(writer, handler, extract(handler, listing)); first = false; count++;
            if ((count % 250) == 0) println("Exported semantic features for " + count + " handler targets");
        }
        writer.write("]}"); writer.close();
        println("Exported " + count + " opcode-handler semantic feature records to " + output.getAbsolutePath());
    }
}
