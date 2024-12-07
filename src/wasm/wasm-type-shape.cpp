/*
 * Copyright 2024 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "wasm-type-shape.h"
#include "support/hash.h"
#include "wasm-type.h"

namespace wasm {

namespace {

enum Comparison { EQ, LT, GT };

template<typename CompareTypes> struct RecGroupComparator {
  std::unordered_map<HeapType, Index> indicesA;
  std::unordered_map<HeapType, Index> indicesB;
  CompareTypes compareTypes;

  RecGroupComparator(CompareTypes compareTypes) : compareTypes(compareTypes) {}

  Comparison compare(const RecGroupShape& a, const RecGroupShape& b) {
    if (a.types.size() != b.types.size()) {
      return a.types.size() < b.types.size() ? LT : GT;
    }
    // Initialize index maps.
    for (auto type : a.types) {
      indicesA.insert({type, indicesA.size()});
    }
    for (auto type : b.types) {
      indicesB.insert({type, indicesB.size()});
    }
    // Compare types until we find a difference.
    for (size_t i = 0; i < a.types.size(); ++i) {
      auto cmp = compareDefinition(a.types[i], b.types[i]);
      if (cmp == EQ) {
        continue;
      }
      return cmp;
    }
    // Never found a difference.
    return EQ;
  }

  Comparison compareDefinition(HeapType a, HeapType b) {
    if (a.isShared() != b.isShared()) {
      return a.isShared() < b.isShared() ? LT : GT;
    }
    if (a.isOpen() != b.isOpen()) {
      return a.isOpen() < b.isOpen() ? LT : GT;
    }
    auto aSuper = a.getDeclaredSuperType();
    auto bSuper = b.getDeclaredSuperType();
    if (aSuper.has_value() != bSuper.has_value()) {
      return aSuper.has_value() < bSuper.has_value() ? LT : GT;
    }
    if (aSuper) {
      if (auto cmp = compare(*aSuper, *bSuper); cmp != EQ) {
        return cmp;
      }
    }
    auto aKind = a.getKind();
    auto bKind = b.getKind();
    if (aKind != bKind) {
      return aKind < bKind ? LT : GT;
    }
    switch (aKind) {
      case HeapTypeKind::Func:
        return compare(a.getSignature(), b.getSignature());
      case HeapTypeKind::Struct:
        return compare(a.getStruct(), b.getStruct());
      case HeapTypeKind::Array:
        return compare(a.getArray(), b.getArray());
      case HeapTypeKind::Cont:
        assert(a.isContinuation() && b.isContinuation());
        return compare(a.getContinuation(), b.getContinuation());
      case HeapTypeKind::Basic:
        break;
    }
    WASM_UNREACHABLE("unexpected kind");
  }

  Comparison compare(Signature a, Signature b) {
    if (auto cmp = compare(a.params, b.params); cmp != EQ) {
      return cmp;
    }
    return compare(a.results, b.results);
  }

  Comparison compare(const Struct& a, const Struct& b) {
    if (a.fields.size() != b.fields.size()) {
      return a.fields.size() < b.fields.size() ? LT : GT;
    }
    for (size_t i = 0; i < a.fields.size(); ++i) {
      if (auto cmp = compare(a.fields[i], b.fields[i]); cmp != EQ) {
        return cmp;
      }
    }
    return EQ;
  }

  Comparison compare(Array a, Array b) { return compare(a.element, b.element); }

  Comparison compare(Continuation a, Continuation b) {
    return compare(a.type, b.type);
  }

  Comparison compare(Field a, Field b) {
    if (a.mutable_ != b.mutable_) {
      return a.mutable_ < b.mutable_ ? LT : GT;
    }
    if (a.isPacked() != b.isPacked()) {
      return b.isPacked() < a.isPacked() ? LT : GT;
    }
    if (a.packedType != b.packedType) {
      return a.packedType < b.packedType ? LT : GT;
    }
    return compare(a.type, b.type);
  }

  Comparison compare(Type a, Type b) {
    if (a.isBasic() != b.isBasic()) {
      return b.isBasic() < a.isBasic() ? LT : GT;
    }
    if (a.isBasic()) {
      if (a.getBasic() != b.getBasic()) {
        return a.getBasic() < b.getBasic() ? LT : GT;
      }
      return EQ;
    }
    if (a.isTuple() != b.isTuple()) {
      return a.isTuple() < b.isTuple() ? LT : GT;
    }
    if (a.isTuple()) {
      return compare(a.getTuple(), b.getTuple());
    }
    assert(a.isRef() && b.isRef());
    if (a.isNullable() != b.isNullable()) {
      return a.isNullable() < b.isNullable() ? LT : GT;
    }
    return compare(a.getHeapType(), b.getHeapType());
  }

  Comparison compare(const Tuple& a, const Tuple& b) {
    if (a.size() != b.size()) {
      return a.size() < b.size() ? LT : GT;
    }
    for (size_t i = 0; i < a.size(); ++i) {
      if (auto cmp = compare(a[i], b[i]); cmp != EQ) {
        return cmp;
      }
    }
    return EQ;
  }

  Comparison compare(HeapType a, HeapType b) {
    if (a.isBasic() != b.isBasic()) {
      return b.isBasic() < a.isBasic() ? LT : GT;
    }
    if (a.isBasic()) {
      if (a.getID() != b.getID()) {
        return a.getID() < b.getID() ? LT : GT;
      }
      return EQ;
    }
    auto itA = indicesA.find(a);
    auto itB = indicesB.find(b);
    bool foundA = itA != indicesA.end();
    bool foundB = itB != indicesB.end();
    if (foundA != foundB) {
      return foundB < foundA ? LT : GT;
    }
    if (foundA) {
      auto indexA = itA->second;
      auto indexB = itB->second;
      if (indexA != indexB) {
        return indexA < indexB ? LT : GT;
      }
      return EQ;
    }
    // These types are external to the group, so fall back to the provided
    // comparator.
    return compareTypes(a, b);
  }
};

// Deduction guide to satisfy -Wctad-maybe-unsupported.
template<typename CompareTypes>
RecGroupComparator(CompareTypes) -> RecGroupComparator<CompareTypes>;

struct RecGroupHasher {
  std::unordered_map<HeapType, Index> typeIndices;

  size_t hash(const RecGroupShape& shape) {
    for (auto type : shape.types) {
      typeIndices.insert({type, typeIndices.size()});
    }
    size_t digest = wasm::hash(shape.types.size());
    for (auto type : shape.types) {
      hash_combine(digest, hashDefinition(type));
    }
    return digest;
  }

  size_t hashDefinition(HeapType type) {
    size_t digest = wasm::hash(type.isShared());
    wasm::rehash(digest, type.isOpen());
    auto super = type.getDeclaredSuperType();
    wasm::rehash(digest, super.has_value());
    if (super) {
      hash_combine(digest, hash(*super));
    }
    auto kind = type.getKind();
    // Mix in very random numbers to differentiate the kinds.
    switch (kind) {
      case HeapTypeKind::Func:
        wasm::rehash(digest, 1904683903);
        hash_combine(digest, hash(type.getSignature()));
        return digest;
      case HeapTypeKind::Struct:
        wasm::rehash(digest, 3273309159);
        hash_combine(digest, hash(type.getStruct()));
        return digest;
      case HeapTypeKind::Array:
        wasm::rehash(digest, 4254688366);
        hash_combine(digest, hash(type.getArray()));
        return digest;
      case HeapTypeKind::Cont:
        assert(type.isContinuation());
        wasm::rehash(digest, 2381496927);
        hash_combine(digest, hash(type.getContinuation()));
        return digest;
      case HeapTypeKind::Basic:
        break;
    }
    WASM_UNREACHABLE("unexpected kind");
  }

  size_t hash(Signature sig) {
    size_t digest = hash(sig.params);
    hash_combine(digest, hash(sig.results));
    return digest;
  }

  size_t hash(const Struct& struct_) {
    size_t digest = wasm::hash(struct_.fields.size());
    for (auto field : struct_.fields) {
      hash_combine(digest, hash(field));
    }
    return digest;
  }

  size_t hash(Array array) { return hash(array.element); }

  size_t hash(Continuation cont) { return hash(cont.type); }

  size_t hash(Field field) {
    size_t digest = wasm::hash(field.mutable_);
    wasm::rehash(digest, field.packedType);
    hash_combine(digest, hash(field.type));
    return digest;
  }

  size_t hash(Type type) {
    size_t digest = wasm::hash(type.isBasic());
    if (type.isBasic()) {
      wasm::rehash(digest, type.getBasic());
      return digest;
    }
    wasm::rehash(digest, type.isTuple());
    if (type.isTuple()) {
      hash_combine(digest, hash(type.getTuple()));
      return digest;
    }
    assert(type.isRef());
    wasm::rehash(digest, type.isNullable());
    hash_combine(digest, hash(type.getHeapType()));
    return digest;
  }

  size_t hash(const Tuple& tuple) {
    size_t digest = wasm::hash(tuple.size());
    for (auto type : tuple) {
      hash_combine(digest, hash(type));
    }
    return digest;
  }

  size_t hash(HeapType type) {
    size_t digest = wasm::hash(type.isBasic());
    if (type.isBasic()) {
      wasm::rehash(digest, type.getID());
      return digest;
    }
    auto it = typeIndices.find(type);
    wasm::rehash(digest, it != typeIndices.end());
    if (it != typeIndices.end()) {
      wasm::rehash(digest, it->second);
      return digest;
    }
    wasm::rehash(digest, type.getID());
    return digest;
  }
};

Comparison compareComparable(const ComparableRecGroupShape& a,
                             const RecGroupShape& b) {
  return RecGroupComparator{[&](HeapType ht1, HeapType ht2) {
           return a.less(ht1, ht2) ? LT : a.less(ht2, ht1) ? GT : EQ;
         }}
    .compare(a, b);
}

} // anonymous namespace

bool RecGroupShape::operator==(const RecGroupShape& other) const {
  return EQ == RecGroupComparator{[](HeapType a, HeapType b) {
                 return a == b ? EQ : LT;
               }}.compare(*this, other);
}

bool ComparableRecGroupShape::operator<(const RecGroupShape& other) const {
  return LT == compareComparable(*this, other);
}

bool ComparableRecGroupShape::operator>(const RecGroupShape& other) const {
  return GT == compareComparable(*this, other);
}

} // namespace wasm

namespace std {

size_t
hash<wasm::RecGroupShape>::operator()(const wasm::RecGroupShape& shape) const {
  return wasm::RecGroupHasher{}.hash(shape);
}

} // namespace std
