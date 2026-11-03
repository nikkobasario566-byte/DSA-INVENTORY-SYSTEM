# Your Perfume Shop: Online Store

A single-page online storefront for the perfume business. It mirrors the catalog
of the C++ inventory system (`dsa-final.cpp`) so customers can browse and order
online while the shop owner manages stock in the inventory system.

**File:** `perfume_store.html` (one self-contained file, no installation needed)

## 1. Features
- Product catalog of 15 perfumes with brand, category, price (PHP) and live stock status
- Search by name, brand or category
- Filter by official partner brand (Dior, Chanel, Versace, Calvin Klein, Giorgio Armani, Yves Saint Laurent, Victoria's Secret)
- Shopping cart with quantity controls; customers cannot add more than the available stock
- Checkout form: customer name, delivery address and contact number (validated, 7 to 15 digits)
- Stock labels: In stock, Only N left (5 or fewer), Out of stock
- Automatic light and dark mode, and a layout that works on phones and desktops
- After an order is placed, the page produces a one-line order record that can be copied

## 2. How to Run
Open `perfume_store.html` in any web browser (Chrome, Edge, Firefox). It works
offline and needs no server.

## 3. Link to the Inventory System
The store uses the same product IDs (101 to 115), names, brands, prices and
starting stock as `seed()` in `dsa-final.cpp`.

When a customer places an order, the store shows an order line in this format:

    ORDER|1001|Customer Name|Delivery Address|Contact Number|101:1;105:2

| Part | Meaning |
|------|---------|
| ORDER | Record type |
| 1001 | Order number |
| Customer Name, Delivery Address, Contact Number | Details entered at checkout |
| 101:1;105:2 | Items as ProductID:Quantity, separated by semicolons |

**Processing an online order (current workflow)**
1. Click **Copy order line** and send it to the shop owner.
2. In the inventory system, choose **5** and add each item by Product ID and quantity.
3. Enter the customer name, delivery address and contact number when placing the order.
4. Choose **6** to process the queue. Stock is deducted and the receipt prints.

## 4. Keeping the Two Systems in Sync
- The inventory system is the master record of stock.
- If you change prices, brands or stock in the inventory system (menu 11), update the product list in the `D` array near the bottom of `perfume_store.html` so both match.
- Each row in `D` is: `[id, name, category, brand, price, stock]`.

## 5. Customizing
- Store name: edit the `Your Perfume Shop` text in the header and the page title.
- Products: edit the `D` array.
- Colors: edit the CSS variables at the top of the file (`--accent`, `--bg`, and so on).

## 6. Limitations
- This is a front-end demo. There is no server or database, so orders are not saved and shown stock resets when the page is reloaded.
- Orders reach the inventory system by copying the order line and entering it manually. The C++ program does not read the order line automatically.
- There is no online payment. Payment (cash on delivery, bank transfer or e-wallet) must be arranged separately.
- Product images are illustrated placeholders. Real photos need hosting and can be added later.

## 7. Possible Future Work
- Add an "Import online order" option to the C++ program that reads the order line directly into the checkout queue.
- Save and load the inventory to a file so both systems read the same data.
- Connect the store to a real marketplace or hosted platform such as Shopee, Lazada, TikTok Shop or Shopify.
