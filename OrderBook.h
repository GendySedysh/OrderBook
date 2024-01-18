#pragma once

#include "LogDuration.h"
#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>

enum class BookType {
	BUY,
	SELL
};

struct Book
{
	unsigned int id;
	unsigned int price;
	unsigned int volume;
	BookType type;
};

class OrderBook
{
private:
	std::map<int, Book> id_to_book_; // для изменений
	std::map<int, int> price_to_volume_; // для вывода

public:
	OrderBook() {}
	~OrderBook() {}

	void AddBook(Book new_book) {
		if (id_to_book_.count(new_book.id) != 0) {
			return;
		}

		int price = new_book.price;
		if (new_book.type == BookType::SELL) {
			price *= -1;
		}

		id_to_book_[new_book.id] = new_book;
		price_to_volume_[price] += new_book.volume;
	}

	void DeleteBook(int book_id) {
		if (id_to_book_.count(book_id) != 0) {
			Book book = id_to_book_.at(book_id);

			int tmp_price = book.price;
			if (book.type == BookType::SELL) {
				tmp_price *= -1;
			}

			price_to_volume_[tmp_price] -= book.volume;
			if (price_to_volume_.at(tmp_price) <= 0) {
				price_to_volume_.erase(tmp_price);
			}
			id_to_book_.erase(book_id);
		}
	}

	void ChangeBook(unsigned int book_id, unsigned int new_price, unsigned int new_volume, BookType new_type) {
		if (id_to_book_.count(book_id) != 0) {
			DeleteBook(book_id);
			AddBook(Book{ book_id, new_price, new_volume, new_type });
		}
	}

	void ShowBestBooks() {
		std::vector<int> prices = std::move(GetBestPrices());

		for (auto& price : prices) {
			std::string type = "BUY";
			if (price < 0) {
				type = "SELL";
			}

			std::cout << "PRICE: " << std::abs(price) <<
				", VOLUME: " << price_to_volume_[price] <<
				", TO " << type << std::endl;
		}
	}

	std::vector<int> GetBestPrices() const {
		std::vector<int> to_return;
		std::vector<std::pair<int, int>> tmp_price_to_volume;
		for (auto& pair : price_to_volume_) {
			tmp_price_to_volume.push_back(pair);
		}

		std::sort(tmp_price_to_volume.begin(), tmp_price_to_volume.end(), 
			[](std::pair<int, int> lhs, std::pair<int, int> rhs) {
				return (std::abs(lhs.first * lhs.second) > std::abs(rhs.first * rhs.second));
			});
		
		for (auto& [price, volume] : tmp_price_to_volume) {
			to_return.push_back(price);

			if (to_return.size() == 10) {
				break;
			}
		}

		return to_return;
	}

	std::map<int, int> GetPriceMap() const {
		return price_to_volume_;
	}
};

class BookTester
{
public:
	void TestAddBook() {
		{
			LogDuration test("Add test 1");
			OrderBook book;
			book.AddBook(Book{ 1, 100, 23, BookType::BUY });
		}

		{
			LogDuration test("Add test 1000");
			OrderBook book;
			for (unsigned int i = 0; i < 1000; i++) {
				book.AddBook(Book{ i, 100, 23, BookType::BUY });
			}
		}

		{
			LogDuration test("Add test 1000000");
			OrderBook book;
			for (unsigned int i = 0; i < 1000000; i++) {
				book.AddBook(Book{ i, 100, 23, BookType::BUY });
			}
		}

		{
			OrderBook book;
			book.AddBook(Book{ 1, 100, 23, BookType::BUY });
			book.AddBook(Book{ 2, 200, 63, BookType::BUY });
			book.AddBook(Book{ 3, 150, 55, BookType::SELL });

			std::map<int, int> tmp_map = { { -150, 55 }, { 100, 23 }, { 200, 63 } };
			assert(book.GetPriceMap() == tmp_map);

			book.AddBook(Book{ 4, 150, 5, BookType::SELL });
			book.AddBook(Book{ 5, 150, 47, BookType::BUY });

			tmp_map = { { -150, 60 }, { 100, 23 }, { 150, 47 }, { 200, 63 } };
			assert(book.GetPriceMap() == tmp_map);

			book.AddBook(Book{ 6, 100, 23, BookType::BUY });
			book.AddBook(Book{ 7, 100, 27, BookType::BUY });
			book.AddBook(Book{ 8, 100, 30, BookType::BUY });

			tmp_map = { { -150, 60 }, { 100, 103 }, { 150, 47 }, { 200, 63 } };
			assert(book.GetPriceMap() == tmp_map);
		}
	}

	void TestDeleteBook() {
		{
			OrderBook book;
			book.AddBook(Book{ 1, 100, 23, BookType::BUY });
			{
				LogDuration test("Delete test 1");
				book.DeleteBook(1);
			}
		}

		{
			OrderBook book;
			for (unsigned int i = 0; i < 1000; i++) {
				book.AddBook(Book{ i, 100, 23, BookType::BUY });
			}

			{
				LogDuration test("Delete test 1000");
				for (unsigned int i = 0; i < 1000; i++) {
					book.DeleteBook(i);
				}
			}
		}

		{
			OrderBook book;
			for (unsigned int i = 0; i < 1000000; i++) {
				book.AddBook(Book{ i, 100, 23, BookType::BUY });
			}
			{
				LogDuration test("Delete test 1000000");
				for (unsigned int i = 0; i < 1000000; i++) {
					book.DeleteBook(i);
				}
			}
		}

		{
			OrderBook book;
			book.AddBook(Book{ 1, 100, 23, BookType::BUY });
			book.AddBook(Book{ 2, 200, 63, BookType::BUY });
			book.AddBook(Book{ 3, 150, 55, BookType::SELL });

			std::map<int, int> tmp_map = { { -150, 55 }, { 100, 23 }, { 200, 63 } };
			assert(book.GetPriceMap() == tmp_map);

			book.DeleteBook(2);

			tmp_map = { { -150, 55 }, { 100, 23 } };
			assert(book.GetPriceMap() == tmp_map);

			book.AddBook(Book{ 6, 100, 23, BookType::BUY });
			book.AddBook(Book{ 7, 100, 27, BookType::BUY });
			book.AddBook(Book{ 8, 100, 30, BookType::BUY });

			tmp_map = { { -150, 55 }, { 100, 103 } };
			assert(book.GetPriceMap() == tmp_map);

			book.DeleteBook(8);
			book.DeleteBook(8);
			book.DeleteBook(6);

			tmp_map = { { -150, 55 }, { 100, 50 } };
			assert(book.GetPriceMap() == tmp_map);
		}
	}

	void TestChangeBook() {

		{
			OrderBook book;
			book.AddBook(Book{ 1, 100, 23, BookType::BUY });
			{
				LogDuration test("Change test 1");
				book.ChangeBook(1, 55, 63, BookType::SELL);
			}
		}

		{
			OrderBook book;
			book.AddBook(Book{ 1, 100, 23, BookType::BUY });
			{
				LogDuration test("Change test 1000");
				for (unsigned int i = 0; i < 1000; i++) {
					book.ChangeBook(1, i, i, BookType::SELL);
				}
			}
		}

		{
			OrderBook book;
			book.AddBook(Book{ 1, 100, 23, BookType::BUY });
			{
				LogDuration test("Change test 1000000");
				for (unsigned int i = 0; i < 1000000; i++) {
					book.ChangeBook(1, i, i, BookType::SELL);
				}
			}
		}

		{
			OrderBook book;
			book.AddBook(Book{ 1, 100, 23, BookType::BUY });

			std::map<int, int> tmp_map = { { 100, 23 } };
			assert(book.GetPriceMap() == tmp_map);

			book.ChangeBook(1, 200, 54, BookType::BUY);
			tmp_map = { { 200, 54 } };
			assert(book.GetPriceMap() == tmp_map);

			book.ChangeBook(1, 200, 54, BookType::SELL);
			tmp_map = { { -200, 54 } };
			assert(book.GetPriceMap() == tmp_map);

			book.AddBook(Book{ 2, 200, 63, BookType::BUY });
			book.AddBook(Book{ 3, 150, 55, BookType::SELL });
			book.AddBook(Book{ 4, 200, 15, BookType::SELL });
			tmp_map = { { -200, 69 }, { -150, 55 }, { 200, 63 } };
			assert(book.GetPriceMap() == tmp_map);
		}
	}

	void TestShowBook() {
		{
			OrderBook book;
			for (unsigned int i = 0; i < 1000000; i++) {
				book.AddBook(Book{ i, 100, 23, BookType::BUY });
			}

			{
				LogDuration test("GetBestPrices test 1");
				book.GetBestPrices();
			}

			{
				LogDuration test("GetBestPrices test 1000");
				for (unsigned int i = 0; i < 1000; i++) {
					book.GetBestPrices();
				}
			}

			{
				LogDuration test("GetBestPrices test 1000000");
				for (unsigned int i = 0; i < 1000000; i++) {
					book.GetBestPrices();
				}
			}
		}

		{
			OrderBook book;
			book.AddBook(Book{ 1, 100, 23, BookType::BUY });
			book.AddBook(Book{ 2, 200, 63, BookType::BUY });
			book.AddBook(Book{ 3, 150, 55, BookType::SELL });

			std::vector<int> tmp_prices = { 200, -150, 100 };
			assert(book.GetBestPrices() == tmp_prices);

			book.AddBook(Book{ 4, 1, 100, BookType::BUY });
			book.AddBook(Book{ 5, 2, 100, BookType::BUY });
			book.AddBook(Book{ 6, 3, 100, BookType::BUY });
			book.AddBook(Book{ 7, 4, 100, BookType::BUY });
			book.AddBook(Book{ 8, 5, 100, BookType::BUY });
			book.AddBook(Book{ 9, 6, 100, BookType::BUY });
			book.AddBook(Book{ 10, 7, 100, BookType::BUY });
			book.AddBook(Book{ 11, 8, 100, BookType::BUY });
			book.AddBook(Book{ 12, 9, 100, BookType::BUY });

			tmp_prices = { 200, -150, 100, 9, 8, 7, 6, 5, 4, 3 };
			assert(book.GetBestPrices() == tmp_prices);

			book.AddBook(Book{ 13, 1, 100000, BookType::BUY });
			book.AddBook(Book{ 14, 2, 65000, BookType::BUY });
			book.AddBook(Book{ 15, 3, 55000, BookType::BUY });

			tmp_prices = { 3, 2, 1, 200, -150, 100, 9, 8, 7, 6 };
			assert(book.GetBestPrices() == tmp_prices);
		}
	}
};

