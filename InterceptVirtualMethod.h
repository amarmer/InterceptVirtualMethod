#pragma once

#include "windows.h"


template <int n>
struct VTable : public VTable<n - 1>
{
    virtual int STDMETHODCALLTYPE GetIndex(VTable*) { return n; }
};


template <>
struct VTable<0>
{
    virtual int STDMETHODCALLTYPE GetIndex(VTable*) { return 0; }
};

typedef VTable<50> MaxVTable;


template <typename T>
int GetVTableSize()
{
    struct VTableSizeDetector : T
    {
        virtual int STDMETHODCALLTYPE GetSize(void* p = nullptr) { return -1; }
    };

    return reinterpret_cast<VTableSizeDetector*>(&MaxVTable())->GetSize();
};


template <typename T, typename TRet, typename... Args>
int GetVTableIndex(TRet(STDMETHODCALLTYPE T::*f)(Args...))
{
    MaxVTable vt;
    T* pT = reinterpret_cast<T*>(&vt);

    auto fGetIndex = (int (STDMETHODCALLTYPE T::*)(void*))f;

    return (pT->*fGetIndex)(nullptr);
}



template <typename T, typename TF, typename TRet, typename... Args>
TRet(STDMETHODCALLTYPE *SetVTableFunction(T* pT, TRet(STDMETHODCALLTYPE TF::*fOrig)(Args...), TRet(STDMETHODCALLTYPE *fNew)(T*, Args...), bool protect = true))(T*, Args...)
{
    // Check that T is derived from TF
    TF* p = pT;

    if (!pT)
    {
        return nullptr;
    }

    if (!fNew)
    {
        return nullptr;
    }

    typedef void (STDMETHODCALLTYPE *VTableFunction)();

    struct VTable
    {
        VTableFunction* pVTableFunctions;
    };

    VTable* pVTable = reinterpret_cast<VTable*>(pT);

    int indx = GetVTableIndex(fOrig);

    auto fOld = (TRet(STDMETHODCALLTYPE*)(T*, Args...))*(pVTable->pVTableFunctions + indx);

    if (fOld == fNew)
    {
        return fNew;
    }

    DWORD oldProtect = 0;
    if (protect)
    {
        if (!::VirtualProtect(pVTable->pVTableFunctions + indx, sizeof(VTableFunction), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            // Log Error

            return nullptr;
        }
    }

    *(pVTable->pVTableFunctions + indx) = reinterpret_cast<VTableFunction>(fNew);

    if (protect)
    {
        if (!::VirtualProtect(pVTable->pVTableFunctions + indx, sizeof(VTableFunction), oldProtect, &oldProtect))
        {
            // Log Error
        }
    }

    return fOld;
}



template <typename T, typename TRet, typename... Args>
TRet(STDMETHODCALLTYPE *VTableFunctionType(TRet(STDMETHODCALLTYPE T::*fOrig)(Args...)))(T*, Args...);

#define VTABLE_FUNCTION_TYPE(T, F) decltype(VTableFunctionType<T>(&T::F))


/*
VTableProtection can be used when shimming several functions, then in SetVTableFunction last parameter "protect" should false, for instance:

VTableProtection protect(pIFileDialog);
auto fShow = SetVTableFunction(pIFileDialog, &IFileDialog::Show, false);
auto fGetResult = SetVTableFunction(pIFileDialog, &IFileDialog::GetResult, false);
protect.RestoreProtection();
*/
class VTableProtection
{
    void* pVTableFunctions_;
    int tableSize_;
    bool protectionRestored_ = true;
    DWORD oldProtect_;
    bool ok_ = true;
private:
    typedef void (STDMETHODCALLTYPE *VTableFunction)();

    struct VTable
    {
        VTableFunction* pVTableFunctions;
    };

public:
    template <typename T>
    VTableProtection(T* pT)
        : pVTableFunctions_(reinterpret_cast<VTable*>(pT)->pVTableFunctions), tableSize_(GetVTableSize<T>())
    {
        if (!::VirtualProtect(pVTableFunctions_, sizeof(VTableFunction)*tableSize_, PAGE_EXECUTE_READWRITE, &oldProtect_))
        {
            // Log Error

            ok_ = false;
            return;
        }
    }

    ~VTableProtection()
    {
        if (!ok_)
            return;

        if (protectionRestored_)
            return;

        RestoreProtection();
    }

    void RestoreProtection()
    {
        if (ok_)
            return;

        if (!::VirtualProtect(pVTableFunctions_, sizeof(VTableFunction)*tableSize_, oldProtect_, &oldProtect_))
        {
            // Log Error
        }

        protectionRestored_ = true;
    }

    operator bool()
    {
        return ok_;
    }
};

