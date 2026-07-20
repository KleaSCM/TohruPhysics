/**
 * Static allocation guard for TohruPhysics.
 * TohruPhysicsの静的確保ガードよ。
 *
 * Forbids operator new/delete at compile time — any use in library code
 * will produce a linker error. Hot paths must never heap-allocate.
 * コンパイル時にoperator new/deleteを禁止するの。ライブラリコードでの使用は
 * リンクエラーになるわ。ホットパスでは絶対にヒープ確保しちゃダメよ。
 *
 * Include this in any translation unit that must remain heap-free.
 * ヒープフリーにしなきゃいけない翻訳単位でインクルードしてね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stddef.h>

// C++ replacements — deleted at global scope.
// グローバルスコープで削除するC++の置き換え。
void *operator new(size_t) = delete;
void *operator new[](size_t) = delete;
void operator delete(void *) noexcept = delete;
void operator delete[](void *) noexcept = delete;
void operator delete(void *, size_t) noexcept = delete;
void operator delete[](void *, size_t) noexcept = delete;

// Placement new is also forbidden — no in-place construction either.
// プレースメントnewも禁止 — その場構築もなしよ。
void *operator new(size_t, void *) = delete;
void *operator new[](size_t, void *) = delete;
