#pragma once

#include "windows.h"

template <int n>
struct VTable : public VTable<n - 1> {
  virtual int GetIndex(VTable*) { return n; }
};

template <>
struct VTable<0> {
  virtual int GetIndex(VTable*) { return 0; }
};

// Assume that there is no more than 50 virtual functions in a class.
using MaxVTable = VTable<50>;

template <typename T>
int GetVTableSize() {
  struct VTableSizeDetector : T {
    virtual int GetSize(void* p = nullptr) { return -1; }
  };

  MaxVTable maxVTable;

  return reinterpret_cast<VTableSizeDetector*>(&maxVTable)->GetSize();
};

template <typename T, typename TRet, typename... Args>
int GetVTableIndex(TRet(T::* f)(Args...)) {
  auto fGetIndex = reinterpret_cast<int (T::*)(void*)>(f);

  MaxVTable maxVTable;

  return (reinterpret_cast<T*>(&maxVTable)->*fGetIndex)(nullptr);
}

template <typename T, typename TF, typename TRet, typename... Args>
TRet (*OverrideVTableFunction(T* pT, TRet(TF::* fOrig)(Args...), TRet(*fNew)(T*, Args...)))(T*, Args...) {
  // Check that T is derived from TF
  TF* p = pT;

  if (!pT)
    return nullptr;

  if (!fNew)
    return nullptr;

  using VTableFunction = void (*)();

  struct VTable {
    VTableFunction* pVTableFunctions;
  };

  auto pVTable = reinterpret_cast<VTable*>(pT);

  int indx = GetVTableIndex(fOrig);

  auto fOld = reinterpret_cast<decltype(fNew)>(*(pVTable->pVTableFunctions + indx));

  if (fOld == fNew)
    return fNew;

  DWORD oldProtect = 0;
  if (!::VirtualProtect(pVTable->pVTableFunctions + indx, sizeof(VTableFunction), PAGE_EXECUTE_READWRITE, &oldProtect)) {
    // Log Error
    return nullptr;
  }

  *(pVTable->pVTableFunctions + indx) = reinterpret_cast<VTableFunction>(fNew);

  if (!::VirtualProtect(pVTable->pVTableFunctions + indx, sizeof(VTableFunction), oldProtect, &oldProtect)) {
    // Log Error
  }

  return fOld;
}

template <typename T, typename TRet, typename... Args>
TRet(*VTableFunctionType(TRet(T::* fOrig)(Args...)))(T*, Args...);

#define VTABLE_FUNCTION_TYPE(F) decltype(VTableFunctionType(F))
