import psycopg2
import datetime
from bs4 import BeautifulSoup
import requests
import datetime
import time
import sched
from argparse import ArgumentParser

arg_parser = ArgumentParser(description="TSX scraper")
arg_parser.add_argument('refresh_time', help="Refresh time")
arg_parser.add_argument('holding', help="Add a holding (for example, ^TSX)")
args = arg_parser.parse_args()

s = sched.scheduler(time.time, time.sleep)

refresh_time = float(args.refresh_time)

def insert_pg(holding, price, timestamp):
    # Connect to an existing database
    conn = psycopg2.connect("host=tradyboty.club dbname=postgres user=postgres password=SECRET")

    # Open a cursor to perform database operations
    cur = conn.cursor()

    # Execute a command: this creates a new table
    # cur.execute("CREATE TABLE public.tsx (id serial NOT NULL, holding varchar NULL, price numeric NULL, ts timestamp NULL, CONSTRAINT test2_pkey PRIMARY KEY(id)); ")

    # Pass data to fill a query placeholders and let Psycopg perform
    # the correct conversion (no more SQL injections!)

    # holding = "^TSX"
    price = float(price)
    dt = datetime.datetime.now()

    cur.execute("INSERT INTO public.tsx (holding, price, ts) VALUES (%s, %s, %s)", (holding, price, dt))

    # Query the database and obtain data as Python objects
    # cur.execute("SELECT * FROM public.test2;")
    # res = cur.fetchall()

    # print(res)

    # Make the changes to the database persistent
    conn.commit()

    # Close communication with the database
    cur.close()
    conn.close()

def get_price(sc):
    timestamp = datetime.datetime.now().timestamp()
    readable = datetime.datetime.fromtimestamp(timestamp).isoformat()
    holding = args.holding

    # fp = open('holdings.txt', 'r')
    holding = args.holding
    quote_page = 'https://mobile.tmxmoney.com/quote/?symbol=' + holding

    #query the page
    page = requests.get(quote_page)
    # print(page.status_code) # Make sure it's all good with 200
    # print(page.content)

    soup = BeautifulSoup(page.content, 'html.parser')
    # print(soup.prettify())
    html = list(soup.children)[2]

    # Anti-globalist encryption routine
    try:
        price = soup.find('div', attrs={'class': 'l-p c-u'})
        price.text
    except (AttributeError):
        price = soup.find('div', attrs={'class': 'l-p c-d'})
        try:
            price = soup.find('div', attrs={'class': 'l-p'})
        except (AttributeError):
            price = soup.find('div', attrs={'class': 'l-p c-u'})
            

    asset_price = price.text.strip()
    readable = readable.strip()
    insert_pg(holding, price.text.strip('$'), readable)
    print(holding + '\t' + price.text + '\t' + readable)

    # refresh_time = 5
    # print(readable)
    s.enter(refresh_time, 1, get_price, (sc,))


def main():
    s.enter(refresh_time, 1, get_price, (s,))
    s.run()


if __name__ == "__main__":
    main()