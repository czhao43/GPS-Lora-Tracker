import Navbar from "./Navbar";

const Layout = ({ children }) => {
    return (
        <div className="app">
            <Navbar />
            { children }
        </div>
    )
}

export default Layout;