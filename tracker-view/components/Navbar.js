import Link from 'next/link';

const Navbar = () => {
    return (
        <nav>
            <div className="logo">
                <h1>GPS LoRa Tracker</h1>
            </div>
            <Link href="/"><a>Dashboard</a></Link>
            <Link href="/analytics"><a>Analytics</a></Link>
            <Link href="https://eceseniordesign2021fall.ece.gatech.edu/sd21f45/#about"><a>About</a></Link>
        </nav>
    )
}

export default Navbar;