# Source/ThirdParty

This is the dedicated location for **third-party C++ source code and pre-built
libraries**, following Unreal Engine's standard `Source/ThirdParty` convention.

Any third-party / open-source C++ code, headers, or binary libraries (`.lib`,
`.dll`, `.a`, `.so`, `.dylib`) used by the **WeaponModule** plugin must live
here, each in its own sub-folder, and be declared on the
[`../THIRD_PARTY_SOFTWARE.md`](../THIRD_PARTY_SOFTWARE.md) form.

## Current status

**WeaponModule bundles no third-party C++ code or libraries.**

* Every source file in `Source/WeaponModule/` is original code
  © Silvan Teufel / Teufel-Engineering.com.
* All other code dependencies are standard **Unreal Engine** modules/plugins
  (shipped with the engine, under the Unreal Engine EULA).
* The only **external** dependency is the **RTSUnitTemplate** plugin — a separate
  product by the same author. It is a *required companion plugin*; it is **not**
  vendored/copied into this folder. It is referenced as a normal plugin + module
  dependency (see `../../WeaponModule.uplugin` and
  `../WeaponModule/WeaponModule.Build.cs`).

This folder is therefore intentionally empty of code; it exists to document the
structure and to provide the correct home should any third-party C++ code be
added in the future.
