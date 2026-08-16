# SOLAR Coding Agent Instructions

Bu talimatlar repository üzerinde çalışan tüm coding agent'ları için bağlayıcıdır.

## 1. Project Overview

SOLAR, C++20 tabanlı modüler bir scientific/engineering desktop application'dır.
NASA CSPICE ile gerçek Solar System durumlarını, reference frame'leri ve yönelimleri işler.
İki-cisim astrodynamics, spacecraft propagation ve impulsive maneuver araçları içerir.
OpenGL 4.6 renderer ile Dear ImGui/ImPlot GUI, scientific state'i görselleştirir ve yönetir.
Rocket, mission-planning ve KSP/RSS katmanları gelecekte eklenecektir; henüz mevcut değildir.
## 2. Repository Architecture

- `src/core`: logging, filesystem, time ve ortak tipler; `SimulationClock` bu hedefte derlenir.
- `src/astronomy`: CSPICE wrapper, ephemeris, bodies, orientation ve reference frames.
- `src/astrodynamics`: orbit matematiği, conversions, Kepler propagation ve maneuvers.
- `src/simulation`: `Spacecraft` domain state'i; `src/scene`: astronomical/render-state köprüsü.
- `src/renderer`: OpenGL/GLAD/GLFW rendering; `src/ui`: Dear ImGui panelleri.
- `src/app`: composition root ve main loop; `tests`: Catch2 scientific/regression testleri.
- `shaders`, `config`, `assets/kernels`: runtime resources; `cmake`: dependency/toolchain helpers.
- Mevcut CMake yönü: `core -> astronomy -> astrodynamics -> simulation`.
- `scene` astronomy/core üzerine kurulur; renderer scene/simulation, UI renderer/scene/simulation kullanır.
- `Application` sistemleri birleştirir. Scientific core hiçbir zaman renderer veya UI'a bağlanmamalıdır.
- Gelecekteki rocket/mission modülleri scientific katmanda kalmalı; UI/OpenGL'e bağımlı olmamalıdır.
## 3. Scientific Correctness

- Fizik, propagasyon ve coordinate-transform formüllerini GUI/render koduna koyma.
- Scientific hesaplarda `double`, `glm::dvec*` ve uygun double matrisleri kullan.
- Physical coordinates/state ile float render coordinates'i kesin olarak ayır.
- Her state/query için origin, observer, axis/reference frame ve epoch açık olmalıdır.
- Birimleri API ve isimlerde belirt: mevcut temel convention km, km/s, km^3/s^2, s, radian'dır.
- GUI metre/saniye veya derece gösterebilir; dönüşüm yalnız presentation boundary'de yapılmalıdır.
- Body/radius/distance render scale fiziksel değerleri değiştirmemelidir.
- Scientific sonuçları viewport, mesh veya ölçeklenmiş renderer çıktısından geri türetme.
- Astronomical simulation time'ın tek kaynağı `SimulationClock` olmalıdır.
## 4. C++ Rules

- Proje C++20 olarak yapılandırılmıştır; daha düşük standarda bağımlı kod ekleme.
- RAII ve value semantics tercih et; ownership açık, ömürler deterministik olsun.
- Gereksiz `new`/`delete`, owning raw pointer ve global mutable state kullanma.
- Header'lara yalnız küçük inline/template implementation koy; büyük kodu `.cpp`'de tut.
- Mevcut `solar::*` namespace, `PascalCase` type ve `m_memberName` convention'ını koru.
- Sınıfları tek sorumlulukta tut; yeni god class veya genel amaçlı state dumping-ground oluşturma.
- OpenGL handle'ları ilgili RAII abstraction içinde tut.
## 5. Project Modification Rules

1. Görevle ilgili CMake hedeflerini, header'ları, implementation'ları ve testleri önce incele.
2. Mevcut data flow, ownership ve error-handling convention'ını anla.
3. Çalışan sistemi gereksiz yere yeniden yazma; en dar uygun extension point'i kullan.
4. Aynı işi yapan ikinci subsystem veya uyumsuz scientific type oluşturma.
- Özellikle `astronomy::StateVector`, `SimulationClock`, `BodyDatabase`, reference frames, `RenderTransform`, renderer resource classes ve framebuffer viewport altyapısını tekrar yaratma.
- Third-party kaynaklarını projeye kopyalama; mevcut FetchContent/CSPICE CMake düzenini kullan.
## 6. Build and Test Workflow

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/solar
```

- CMake 3.24+ gerekir; testler varsayılan olarak açıktır ve dependencies FetchContent ile gelir.
- Değişiklikten sonra gerçekten configure/build/test çalıştır; project warning'lerini incele ve düzelt.
- GUI ortamı ve OpenGL 4.6 mevcutsa uygulamayı başlatıp startup/runtime yolunu doğrula.
- Sanitizer gerektiğinde ayrı build'de `-DSOLAR_ENABLE_SANITIZERS=ON` kullan.
- “Dosyalar oluşturuldu, çalışması gerekir” tamamlanmış iş sayılmaz.
## 7. Testing Rules

- Scientific davranış değişikliği aynı commit/task içinde Catch2 testi gerektirir.
- Orbit mechanics, frames/transforms, time, SPICE, propagation ve gelecekteki rocket/mission hesaplarını bağımsız analytical/reference değerler ve anlamlı toleranslarla test et.
- Round-trip, singular/invalid input, negative time, convergence failure ve invariants'ı kapsa.
- İnternet gerektiren test yazma; SPICE testlerinde repository kernel manifestini kullan.
- 3D'de doğru görünmek scientific doğruluk kanıtı değildir.
## 8. Renderer / GUI Rules

- Renderer scientific hesap yapmaz; yalnız hazırlanmış state/geometry'yi görselleştirir.
- GUI authoritative orbit/body/spacecraft state tutmaz; domain sistemlerine command gönderir.
- Offscreen framebuffer -> color texture -> ImGui Viewport mimarisini koru.
- Camera/shortcut input'u ImGui text/item capture kurallarına uymalıdır.
- Framebuffer, shader, mesh, buffer ve texture ownership'ı RAII ile yönetilmelidir.
- Büyük hesapları UI/render frame loop'unda bloklama; gerekirse kontrollü background iş tasarla.
## 9. Error Handling and Logging

- Terminal ve GUI console için mevcut `SOLAR_LOG_INFO/WARN/ERROR` altyapısını kullan.
- Invalid scientific input, SPICE/shader/file/config failure ve numerical non-convergence'ı yutma.
- Fatal initialization hatalarında açık mesajla dur; recoverable hatalarda güvenli sonuç/fallback döndür.
- NaN, infinity, degenerate vector veya invalid OpenGL state'i sessizce render etme.
## 10. Scope and Documentation

- Yalnız istenen kapsamı uygula; gelecekteki özellikler için TODO, placeholder veya dummy ekleme.
- Yeni public subsystem, scientific assumption ya da unit/frame convention eklenirse README'yi güncelle.
- Yorumlar kodun ne yaptığını tekrar etmemeli; nedenini, convention'ı ve scientific assumption'ı açıklamalı.
## 11. Final Validation Checklist

- [ ] Configure/build başarılı, tüm testler geçti ve warning'ler incelendi mi?
- [ ] Mevcut özellikler ve dependency boundaries korundu mu?
- [ ] Units, epoch, origin ve reference frame açık mı?
- [ ] Duplicate type/subsystem veya GUI/render içinde physics oluştu mu?
- [ ] Error paths, ownership ve resource cleanup doğrulandı mı?
- [ ] Uygulama ortam izin verdiğinde başlatıldı ve dokümantasyon güncel mi?
