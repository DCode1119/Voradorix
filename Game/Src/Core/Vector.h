#pragma once
#include <algorithm>
#include <initializer_list>
#include <vector>

template <typename ElementType>
class TVrdxVector
{
public:
	TVrdxVector() = default;
	TVrdxVector(std::initializer_list<ElementType> List) : Container(List) {}

	// --- STL 호환 (범위 기반 for, 알고리즘 등) ---
	ElementType* Data() { return Container.data(); }
	const ElementType* Data() const { return Container.data(); }
	ElementType* Begin() { return Container.data(); }
	const ElementType* Begin() const { return Container.data(); }
	ElementType* End() { return Container.data() + Container.size(); }
	const ElementType* End() const { return Container.data() + Container.size(); }

	// --- 크기 / 용량 ---
	int32_t   Num() const { return static_cast<int32_t>(Container.size()); }
	bool      IsEmpty() const { return Container.empty(); }
	int32_t   Max() const { return static_cast<int32_t>(Container.capacity()); }

	// --- 요소 접근 ---
	ElementType& First() { return Container.front(); }
	const ElementType& First() const { return Container.front(); }
	ElementType& Last() { return Container.back(); }
	const ElementType& Last() const { return Container.back(); }
	ElementType& operator[](int32_t Index) { return Container[Index]; }
	const ElementType& operator[](int32_t Index) const { return Container[Index]; }

	// --- 추가 / 제거 / 검색 ---
	void      Add(const ElementType& Value) { Container.push_back(Value); }
	void      Add(ElementType&& Value) { Container.push_back(std::move(Value)); }
	void      RemoveAt(int32_t Index) { Container.erase(Container.begin() + Index); }
	void      Pop() { Container.pop_back(); }
	void      Clear() { Container.clear(); }
	bool      Contains(const ElementType& Value) const { return std::find(Container.begin(), Container.end(), Value) != Container.end(); }

	// --- 용량 ---
	void      Reserve(int32_t Capacity) { Container.reserve(Capacity); }
	void      Reset(int32_t NewCapacity = 0) { Container.clear(); Container.shrink_to_fit(); if (NewCapacity > 0) Container.reserve(NewCapacity); }

private:
	std::vector<ElementType> Container;
};
