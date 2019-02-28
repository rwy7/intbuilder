#if !defined(OMR_MODEL_VIRTOPERANDSTACK_HPP_)
#define OMR_MODEL_VIRTOPERANDSTACK_HPP_

/**
 * @brief simulates an operand stack used by many bytecode based  machines
 * In such  machines, the operand stack holds the intermediate expression values
 * computed by the bytecodes. The compiler simulates this operand stack as well, but
 * what is pushed to and popped from the simulated operand stack are expression nodes
 * that represent the value computed by the bytecodes. As each bytecode pops expression
 * nodes off the operand stack, it combines them to form more complicated expressions
 * which are then pushed back onto the operand stack for consumption by other bytecodes.
 *
 * The stack is represented as an array of pointers to JB::IlValue's, making it
 * easy to use IlBuilder services to consume and compute new values. Note that,
 * unlike VirtualMachineRegister, the simulated operand stack is *not* maintained
 * by the method code as part of the method's stack frame. This approach requires
 * modelling the state of the operand stack at all program points, which means
 * there cannot only be one VirtualMachineOperandStack object.
 *
 * The current implementation does not share anything among different
 * VirtualMachineOperandStack objects. Possibly, some of the state could be
 * shared to save some memory. For now, simplicity is the goal.
 *
 * VirtualMachineOperandStack implements VirtualMachineState:
 * Commit() simply iterates over the simulated operand stack and stores each
 *   value onto the  machine's operand stack (more details at definition).
 * Reload() is left empty; assumption is that each BytecodeBuilder handler will
 *   update the state of the operand stack appropriately on return from the
 *   interpreter.
 * MakeCopy() copies the state of the operand stack
 * MergeInto() is slightly subtle. Operations may have been already created
 *   below the merge point, and those operations will have assumed the
 *   expressions are stored in the JB::IlValue's for the state being merged
 *   *to*. So the purpose of MergeInto() is to store the values of the current
 *   state into the same variables as in the "other" state.
 * 
 * VirtualMachineOperandStack provides several stack-y operations:
 *   Push() pushes a JB::IlValue onto the stack
 *   Pop() pops and returns a JB::IlValue from the stack
 *   Top() returns the JB::IlValue at the top of the stack
 *   Pick() returns the JB::IlValue "depth" elements from the top
 *   Drop() discards "depth" elements from the stack
 *   Dup() is a convenience function for Push(Top())
 *
 */

#include <OMR/Model/Mode.hpp>
#include <OMR/Model/Value.hpp>
#include <OMR/Model/Register.hpp>

#include <OMR/TypeTraits.hpp>

// #include "ilgen/VirtualMachineState.hpp"
#include "VirtualMachineState.hpp"
#include "IlBuilder.hpp"
#include "TypeDictionary.hpp"

#include <cstdint>
#include <vector>

namespace OMR {
namespace Model {

class VirtOperandStack {
public:
	/*
	VirtOperandStack(const VirtRegister& reg)
		: _reg(reg), {
	}

	JB::IlValue *pick(JB::IlBuilder* b, CUInt depth) {
		return _data.at(depth.value);
	}

	void drop(JB::IlBuilder *b, CUInt depth = 0) {
		_data.
	}

	void dup(JB::IlBuilder *b) {
		push(b, top(b));
	}

	void commit(JB::IlBuilder *b) {
		for ()
	}

	void reload(JB::IlBuilder *b) {

	}

	void overwrite(JB::VirtualMachineOperandStack *other, JB::IlBuilder *b);

	void update(JB::IlBuilder *b, JB::IlValue *stack) {
		_address = address;
	}

	void push(JB::IlBuilder *b, JB::IlValue *value) {
		_data.push_back(value);
	}

	JB::IlValue* peek(JB::IlBuilder& b, CUInt offset) {
		return _data.at(offset);
	}

	JB::IlValue* pop(JB::IlBuilder *b) {
		JB::IlValue* top = top(b);
		_data.pop_back();
		return top;
	}

	JB::IlValue *top(JB::IlBuilder* b) {
		assert(1 < _data.size());
		return _data.at(_data.size() - 1);
	}

   private:

   JB::MethodBuilder *_mb;
   JB::VirtualMachineRegister *_stackTopRegister;
   int32_t _stackMax;
   int32_t _stackTop;
   JB::IlValue **_stack;
   JB::IlType *_elementType;
   int32_t _pushAmount;
   int32_t _stackOffset;
   const char *_stackBaseName;
   */
};

/// grows upwards, store before increment / load after decrement.
///
class RealOperandStack {
public:
	RealOperandStack(JB::IlBuilder* b, JB::IlValue* address)
		: _sp(b->Address, address) {}

	void initialize(JB::IlBuilder* b) {
		_sp.initialize(b);
	}

	void commit(JB::IlBuilder* b) {
		_sp.commit(b);
	}

	void reload(JB::IlBuilder* b) {
		_sp.reload(b);
	}

	void mergeInto(JB::IlBuilder* b, RealOperandStack& dest) {
		_sp.mergeInto(b, dest._sp);
	}

	JB::IlValue* popInt(JB::IlBuilder* b) {
		JB::IlValue* sp = b->Sub(_sp.load(b), constant(b, sizeof(int)));
		JB::IlValue* value = b->LoadAt(b->Int32, sp);
		b->StoreAt(sp, constant(b, std::int32_t(0xdead))); // poison
		_sp.store(b, sp);
		return value;
	}

	void pushInt(JB::IlBuilder* b, JB::IlValue* value) {
		JB::IlValue* sp = _sp.load(b);
		b->StoreAt(sp, value);
		_sp.store(b, b->Add(sp, constant(b, sizeof(int))));
	}

private:
	RealRegister _sp;
};

/// Purely virtual operand stack. No side effects which can be written to the.
class PureOperandStack {
public:

};

template <Mode MODE> struct ModalOperandStackAlias;
template <> struct ModalOperandStackAlias<Mode::REAL> : TypeAlias<RealOperandStack> {};
template <> struct ModalOperandStackAlias<Mode::VIRT> : TypeAlias<VirtOperandStack> {};
template <> struct ModalOperandStackAlias<Mode::PURE> : TypeAlias<PureOperandStack> {};

template <Mode M>
using OperandStack = typename ModalOperandStackAlias<M>::Type;

}  // namespace Model
}  // namespace OMR

#endif // OMR_MODEL_VIRTOPERANDSTACK_HPP_
