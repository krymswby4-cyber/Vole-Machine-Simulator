#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <string>

using namespace std;

// ==================== 1. Class Declarations ====================

class Memory {
private:
    int cells[256] = {0};
public:
    int read(int address);
    void write(int address, int value);
    void clear();
};

class CPU {
private:
    int pc = 0;
    int ir = 0;
    bool halted = false;
    int registers[16] = {0};
public:
    void reset();
    bool isHalted();
    void halt();
    int getPC();
    void setPC(int address);
    int getIR();
    int readRegister(int index);
    void writeRegister(int index, int value);
    void fetch(Memory& memory);
    void cycle(Memory& memory);
};

class Instruction {
public:
    int r = 0;
    int xy = 0;
    virtual void execute(CPU& cpu, Memory& memory) = 0;
    virtual ~Instruction() = default;
};

class LoadValue : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };
class LoadMemory : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };
class Store : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };
class Move : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };
class AddInt : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };
class AddFloat : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };
class BitOr : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };
class BitAnd : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };
class BitXor : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };
class Rotate : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };
class JumpIfEqual : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };
class Halt : public Instruction { public: void execute(CPU& cpu, Memory& memory) override; };

class InstructionDecoder {
public:
    Instruction* decode(int word);
};

class ProgrammLoader {
public:
    void load(string path, Memory& memory, int startAddress);
};

class Display {
public:
    void showState(CPU& cpu, Memory& memory);
    void showMessage(string text);
};

class voleMachine {
private:
    Memory memory;
    CPU cpu;
    ProgrammLoader loader;
    Display display;
public:
    void loadProgram(string path, int startAddress);
    void step();
    void run();
    void reset();
    void showState();
};

// ==================== 2. Implementation ====================

int Memory::read(int address) {
    if (address < 0 || address >= 256) {
        cout << "Error: Address out of bounds!\n";
        return 0;
    }
    return cells[address];
}

void Memory::write(int address, int value) {
    if (address >= 0 && address < 256) {
        cells[address] = value;
    }
}

void Memory::clear() {
    for (int i = 0; i < 256; i++) {
        cells[i] = 0;
    }
}

void CPU::reset() {
    pc = 0;
    ir = 0;
    halted = false;
    for (int i = 0; i < 16; i++) {
        registers[i] = 0;
    }
}

bool CPU::isHalted() {
    return halted;
}

void CPU::halt() {
    halted = true;
}

int CPU::getPC() {
    return pc;
}

void CPU::setPC(int address) {
    pc = address;
}

int CPU::getIR() {
    return ir;
}

int CPU::readRegister(int index) {
    if (index >= 0 && index < 16) {
        return registers[index];
    }
    return 0;
}

void CPU::writeRegister(int index, int value) {
    if (index > 0 && index < 16) {
        registers[index] = value & 0xFF;
    }
}

void CPU::fetch(Memory& memory) {
    if (pc >= 255) {
        halted = true;
        return;
    }
    int highByte = memory.read(pc);
    int lowByte = memory.read(pc + 1);
    ir = (highByte << 8) | lowByte;
    pc += 2;
}

void CPU::cycle(Memory& memory) {
    if (halted) return;

    fetch(memory);

    InstructionDecoder dec;
    Instruction* inst = dec.decode(ir);

    if (inst != nullptr) {
        inst->execute(*this, memory);
        delete inst;
    } else {
        cout << "Error: Invalid Instruction!\n";
        halted = true;
    }
}

float bitToFloat(int val) {
    float f;
    int* p = (int*)&f;
    *p = val;
    return f;
}

int floatToBit(float f) {
    int* p = (int*)&f;
    return *p;
}

void AddInt::execute(CPU& cpu, Memory& memory) {
    int valX = cpu.readRegister((xy >> 4) & 0xF);
    int valY = cpu.readRegister(xy & 0xF);
    cpu.writeRegister(r, (valX + valY) & 0xFF);
}

void AddFloat::execute(CPU& cpu, Memory& memory) {
    float valX = bitToFloat(cpu.readRegister((xy >> 4) & 0xF));
    float valY = bitToFloat(cpu.readRegister(xy & 0xF));
    float result = valX + valY;
    cpu.writeRegister(r, floatToBit(result));
}

void BitOr::execute(CPU& cpu, Memory& memory) {
    int valX = cpu.readRegister((xy >> 4) & 0xF);
    int valY = cpu.readRegister(xy & 0xF);
    cpu.writeRegister(r, valX | valY);
}

void BitAnd::execute(CPU& cpu, Memory& memory) {
    int valX = cpu.readRegister((xy >> 4) & 0xF);
    int valY = cpu.readRegister(xy & 0xF);
    cpu.writeRegister(r, valX & valY);
}

void BitXor::execute(CPU& cpu, Memory& memory) {
    int valX = cpu.readRegister((xy >> 4) & 0xF);
    int valY = cpu.readRegister(xy & 0xF);
    cpu.writeRegister(r, valX ^ valY);
}

void Rotate::execute(CPU& cpu, Memory& memory) {
    int current = cpu.readRegister(r);
    int steps = xy & 0xF;
    int rotated = ((current >> steps) | (current << (8 - steps))) & 0xFF;
    cpu.writeRegister(r, rotated);
}

void JumpIfEqual::execute(CPU& cpu, Memory& memory) {
    if (cpu.readRegister(r) == cpu.readRegister(0)) {
        cpu.setPC(xy);
    }
}

void Halt::execute(CPU& cpu, Memory& memory) {
    cpu.halt();
}

void LoadValue::execute(CPU& cpu, Memory& memory) {
    cpu.writeRegister(r, xy);
}

void LoadMemory::execute(CPU& cpu, Memory& memory) {
    int val = memory.read(xy);
    cpu.writeRegister(r, val);
}

void Store::execute(CPU& cpu, Memory& memory) {
    int val = cpu.readRegister(r);
    memory.write(xy, val);
}

void Move::execute(CPU& cpu, Memory& memory) {
    int val = cpu.readRegister((xy >> 4) & 0xF);
    cpu.writeRegister(r, val);
}

Instruction* InstructionDecoder::decode(int word) {
    int opcode = (word >> 12) & 0xF;
    int r = (word >> 8) & 0xF;
    int xy = word & 0xFF;

    Instruction* inst = nullptr;

    switch (opcode) {
        case 1: inst = new LoadValue(); break;
        case 2: inst = new LoadMemory(); break;
        case 3: inst = new Store(); break;
        case 4: inst = new Move(); break;
        case 5: inst = new AddInt(); break;
        case 6: inst = new AddFloat(); break;
        case 7: inst = new BitOr(); break;
        case 8: inst = new BitAnd(); break;
        case 9: inst = new BitXor(); break;
        case 10: inst = new Rotate(); break;
        case 11: inst = new JumpIfEqual(); break;
        case 13: inst = new Halt(); break;
        default: inst = nullptr; break;
    }

    if (inst != nullptr) {
        inst->r = r;
        inst->xy = xy;
    }

    return inst;
}

void ProgrammLoader::load(string path, Memory& memory, int startAddress) {
    ifstream file(path);
    if (!file.is_open()) {
        cout << "Could not open file: " << path << endl;
        return;
    }

    int address = startAddress;
    string line;
    while (file >> line) {
        int instructionVal = stoi(line, nullptr, 16);

        int high = (instructionVal >> 8) & 0xFF;
        int low = instructionVal & 0xFF;

        memory.write(address, high);
        memory.write(address + 1, low);
        address += 2;
    }
    file.close();
}

void Display::showState(CPU& cpu, Memory& memory) {
    cout << "\n------------------ CPU State ------------------\n";
    cout << "PC: 0x" << hex << cpu.getPC() << dec << "\t" << "Halted: " << (cpu.isHalted() ? "Yes" : "No") << "\n";
    cout << "Registers:\n";
    for (int i = 0; i < 16; i++) {
        cout << "R" << i << ": 0x" << hex << cpu.readRegister(i) << dec << "\t";
        if ((i + 1) % 4 == 0) cout << "\n";
    }
    cout << "-----------------------------------------------\n";
}

void Display::showMessage(string text) {
    cout << "[Message] " << text << "\n";
}

void voleMachine::loadProgram(string path, int startAddress) {
    memory.clear();
    cpu.reset();
    loader.load(path, memory, startAddress);
}

void voleMachine::step() {
    cpu.cycle(memory);
}

void voleMachine::run() {
    while (!cpu.isHalted()) {
        step();
    }
}

void voleMachine::reset() {
    memory.clear();
    cpu.reset();
}

void voleMachine::showState() {
    display.showState(cpu, memory);
}

// ==================== 3. Main Function ====================

int main() {
    voleMachine machine;

    string programFile = "program.txt";

    cout << "Running Vole Machine Simulator...\n";
    machine.loadProgram(programFile, 0);
    machine.run();
    machine.showState();

    return 0;
}