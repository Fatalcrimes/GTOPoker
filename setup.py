from setuptools import setup, find_packages

setup(
    name="poker-gto",
    version="0.1.0",
    description="Poker GTO strategy calculator and optimizer",
    author="Maxim Nguyen",
    author_email="maximngocnguyen.com",
    packages=find_packages(),
    install_requires=[
        "deuces",
        "numpy",
        "matplotlib",  # for visualizations if needed
    ],
    entry_points={
        'console_scripts': [
            'poker-gto=poker_gto.main:main',  # Creates a command-line tool
        ],
    },
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
    ],
    python_requires=">=3.13",
)