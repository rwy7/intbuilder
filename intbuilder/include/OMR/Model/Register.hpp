#if !defined(OMR_MODEL_REGISTER_HPP_)
#define OMR_MODEL_REGISTER_HPP_

#include <OMR/Model/Mode.hpp>
#include <OMR/TypeTraits.hpp>
#include <OMR/Model.hpp>
#include <IlBuilder.hpp>
#include <TypeDictionary.hpp>

namespace OMR {

namespace JB = OMR::JitBuilder;

namespace Model {

/// The real register directly reads and writes.
class RealRegister {
public:
	RealRegister(JB::IlType* type, JB::IlValue* address)
		: _type(type), _address(address) {}

	JB::IlValue* load(JB::IlBuilder* b) { return b->LoadAt(_type, _address); }

	void store(JB::IlBuilder* b, JB::IlValue* value) { b->StoreAt(_address, value); }

	void initialize(JB::IlBuilder* b) {}

	void commit(JB::IlBuilder* b) {}

	void reload(JB::IlBuilder* b) {}

	void mergeInto(JB::IlBuilder*b, RealRegister& dest) {}

private:
	JB::IlType* _type;
	JB::IlValue* _address;
};

class VirtRegister {
public:
	VirtRegister(JB::IlType* type, JB::IlValue* address)
		: _type(type), _address(address), _value(nullptr) {}

	JB::IlValue* load(JB::IlBuilder* b) { return _value; }

	void store(JB::IlBuilder* b, JB::IlValue* value) { _value = value; }

	void initialize(JB::IlBuilder* b) { _value = b->LoadAt(_type, _address); }

	void commit(JB::IlBuilder* b) { b->StoreAt(_address, _value); }

	void reload(JB::IlBuilder* b) { b->StoreOver(_value, b->LoadAt(_type, _address)); }

	void mergeInto(JB::IlBuilder* b, VirtRegister& dest) { b->StoreOver(dest._value, _value); }

private:
	JB::IlType* _type;
	JB::IlValue* _address;
	JB::IlValue* _value;
};

class PureRegister {
public:
	PureRegister(JB::IlType* type)
		: _type(type), _value(nullptr) {}

	JB::IlValue* load(JB::IlBuilder* b) { return _value; }

	void store(JB::IlBuilder* b, JB::IlValue* value) { b->StoreOver(_value, value); }

	void initialize(JB::IlBuilder* b) {
		// HACK: want to use NewValue, but not available through JB APIGen
		_value = b->Const(0);
	}

	void commit(JB::IlBuilder* b) {}

	void reload(JB::IlBuilder* b) {}

	void mergeInto(JB::IlBuilder* b, PureRegister& dest) { b->StoreOver(dest._value, _value); }

private:
	JB::IlType* _type;
	JB::IlValue* _value;
};

template <Mode M> struct ModalRegisterAlias;
template <> struct ModalRegisterAlias<Mode::REAL> : TypeAlias<RealRegister> {};
template <> struct ModalRegisterAlias<Mode::VIRT> : TypeAlias<VirtRegister> {};
template <> struct ModalRegisterAlias<Mode::PURE> : TypeAlias<PureRegister> {};

/**
 * Buffered reads and writes to an address in memory.
 */
template <Mode M> using Register = typename ModalRegisterAlias<M>::Type;

}  // namespace Model
}  // namespace OMR

# endif // OMR_MODEL_REGISTER_HPP_
